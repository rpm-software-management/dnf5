/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

constexpr const char * REPOID_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.:";

#include "repo_cache_private.hpp"
#include "repo_downloader.hpp"
#include "rpm/package_sack_impl.hpp"
#include "solv_repo.hpp"
#include "utils/string.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/conf/const.hpp"
#include "libdnf5/logger/logger.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"

extern "C" {
#include <solv/repo_rpmdb.h>
#include <solv/testcase.h>
}

#include <fcntl.h>
#include <fmt/format.h>
#include <glib.h>
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#include <array>
#include <atomic>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <system_error>
#include <type_traits>


namespace libdnf5::repo {

static void is_readable_rpm(const std::string & fn) {
    if (std::filesystem::path(fn).extension() != ".rpm") {
        throw RepoRpmError(M_("Failed to load RPM \"{}\": doesn't have the \".rpm\" extension"), fn);
    }

    if (access(fn.c_str(), R_OK) != 0) {
        throw RepoRpmError(M_("Failed to access RPM \"{}\": {}"), fn, std::string(strerror(errno)));
    }
}


static int64_t mtime(const char * filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_mtime;
}


Repo::Repo(const BaseWeakPtr & base, const std::string & id, Repo::Type type)
    : base(base),
      config(base->get_config(), id),
      type(type),
      downloader(new RepoDownloader(base, config, type)) {
    if (type == Type::AVAILABLE) {
        auto idx = verify_id(id);
        if (idx != std::string::npos) {
            throw RepoError(M_("Invalid repository id \"{}\": unexpected character '{}'"), id, id[idx]);
        }
    }
    auto & cacheonly = base->get_config().get_cacheonly_option().get_value();
    if (cacheonly != "none") {
        sync_strategy = SyncStrategy::ONLY_CACHE;
    }
}

Repo::Repo(Base & base, const std::string & id, Repo::Type type) : Repo(base.get_weak_ptr(), id, type) {}

Repo::~Repo() = default;

Repo::Type Repo::get_type() const noexcept {
    return type;
}

void Repo::set_callbacks(std::unique_ptr<RepoCallbacks> && callbacks) {
    downloader->set_callbacks(std::move(callbacks));
}

std::unique_ptr<libdnf5::repo::RepoCallbacks> & Repo::get_callbacks() {
    return downloader->callbacks;
}

void Repo::set_user_data(void * user_data) noexcept {
    downloader->set_user_data(user_data);
}

void * Repo::get_user_data() const noexcept {
    return downloader->get_user_data();
}

std::string::size_type Repo::verify_id(const std::string & repo_id) {
    return repo_id.find_first_not_of(REPOID_CHARS);
}

void Repo::verify() const {
    if (config.get_baseurl_option().empty() &&
        (config.get_metalink_option().empty() || config.get_metalink_option().get_value().empty()) &&
        (config.get_mirrorlist_option().empty() || config.get_mirrorlist_option().get_value().empty())) {
        throw RepoError(M_("Repository \"{}\" has no source (baseurl, mirrorlist or metalink) set."), config.get_id());
    }

    const auto & type = config.get_type_option().get_value();
    const char * supported_repo_types[]{"rpm-md", "rpm", "repomd", "rpmmd", "yum", "YUM"};
    if (!type.empty()) {
        for (auto supported : supported_repo_types) {
            if (type == supported) {
                return;
            }
        }
        throw RepoError(M_("Repository \"{}\" has unsupported type \"{}\", skipping."), config.get_id(), type);
    }
}

ConfigRepo & Repo::get_config() noexcept {
    return config;
}

const ConfigRepo & Repo::get_config() const noexcept {
    return config;
}

std::string Repo::get_id() const noexcept {
    return config.get_id();
}

void Repo::enable() {
    config.get_enabled_option().set(Option::Priority::RUNTIME, true);
}

void Repo::disable() {
    config.get_enabled_option().set(Option::Priority::RUNTIME, false);
}

bool Repo::is_enabled() const {
    return config.get_enabled_option().get_value();
}

bool Repo::is_local() const {
    if ((!config.get_metalink_option().empty() && !config.get_metalink_option().get_value().empty()) ||
        (!config.get_mirrorlist_option().empty() && !config.get_mirrorlist_option().get_value().empty())) {
        return false;
    }
    if (!config.get_baseurl_option().get_value().empty() &&
        config.get_baseurl_option().get_value()[0].compare(0, 7, "file://") == 0) {
        return true;
    }
    return false;
}

void Repo::read_metadata_cache() {
    downloader->reset_loaded();
    downloader->load_local();
}


bool Repo::is_in_sync() {
    if (!config.get_metalink_option().empty() && !config.get_metalink_option().get_value().empty()) {
        return downloader->is_metalink_in_sync();
    }
    return downloader->is_repomd_in_sync();
}


void Repo::download_metadata(const std::string & destdir) {
    downloader->download_metadata(destdir);
}

void Repo::load() {
    // Each repository can only be loaded once. This one has already loaded, so just return instantly
    if (loaded) {
        return;
    }

    make_solv_repo();

    if (type == Type::AVAILABLE) {
        load_available_repo();
    } else if (type == Type::SYSTEM) {
        load_system_repo();
    }

    solv_repo->set_needs_internalizing();
    base->get_rpm_package_sack()->p_impl->invalidate_provides();

    loaded = true;
}

void Repo::load_extra_system_repo(const std::string & rootdir) {
    libdnf_user_assert(type == Type::SYSTEM, "repo type must be SYSTEM to load an extra system repo");
    libdnf_user_assert(solv_repo, "repo must be loaded to load an extra system repo");
    solv_repo->load_system_repo(rootdir);
}

void Repo::add_libsolv_testcase(const std::string & path) {
    make_solv_repo();

    libdnf5::utils::fs::File testcase_file(path, "r", true);
    testcase_add_testtags(solv_repo->repo, testcase_file.get(), 0);

    solv_repo->set_needs_internalizing();
    base->get_rpm_package_sack()->p_impl->invalidate_provides();
}

bool Repo::get_use_includes() const {
    return use_includes;
}

void Repo::set_use_includes(bool enabled) {
    use_includes = enabled;
}

int Repo::get_cost() const {
    return config.get_cost_option().get_value();
}

void Repo::set_cost(int value, Option::Priority priority) {
    auto & conf_cost = config.get_cost_option();
    conf_cost.set(priority, value);
    if (solv_repo) {
        solv_repo->set_subpriority(-conf_cost.get_value());
    }
}

int Repo::get_priority() const {
    return config.get_priority_option().get_value();
}

void Repo::set_priority(int value, Option::Priority priority) {
    auto & conf_priority = config.get_priority_option();
    conf_priority.set(priority, value);
    if (solv_repo) {
        solv_repo->set_priority(-conf_priority.get_value());
    }
}

int64_t Repo::get_age() const {
    return time(nullptr) - mtime(downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str());
}

void Repo::expire() {
    expired = true;
}

bool Repo::is_expired() const {
    if (expired)
        // explicitly requested expired state
        return true;
    if (config.get_metadata_expire_option().get_value() == -1)
        return false;
    return get_age() > config.get_metadata_expire_option().get_value();
}

int Repo::get_expires_in() const {
    return config.get_metadata_expire_option().get_value() - static_cast<int>(get_age());
}

void Repo::set_substitutions(const std::map<std::string, std::string> & substitutions) {
    downloader->substitutions = substitutions;
}

std::string Repo::get_metadata_path(const std::string & metadata_type) {
    return downloader->get_metadata_path(metadata_type);
}

void Repo::set_max_mirror_tries(int max_mirror_tries) {
    downloader->max_mirror_tries = max_mirror_tries;
}

int64_t Repo::get_timestamp() const {
    auto path = downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY);
    if (!path.empty()) {
        return mtime(path.c_str());
    }
    return -1;
}

int Repo::get_max_timestamp() {
    return downloader->max_timestamp;
}

void Repo::set_preserve_remote_time(bool preserve_remote_time) {
    downloader->preserve_remote_time = preserve_remote_time;
}

bool Repo::get_preserve_remote_time() const {
    return downloader->preserve_remote_time;
}

const std::vector<std::string> & Repo::get_content_tags() {
    return downloader->content_tags;
}

const std::vector<std::pair<std::string, std::string>> & Repo::get_distro_tags() {
    return downloader->distro_tags;
}

const std::vector<std::pair<std::string, std::string>> Repo::get_metadata_locations() const {
    return downloader->metadata_locations;
}

std::string Repo::get_cachedir() const {
    return config.get_cachedir();
}

std::string Repo::get_persistdir() const {
    return config.get_persistdir();
}

const std::string & Repo::get_revision() const {
    return downloader->revision;
}

void Repo::set_repo_file_path(const std::string & path) {
    repo_file_path = path;
}

const std::string & Repo::get_repo_file_path() const noexcept {
    return repo_file_path;
}

void Repo::set_sync_strategy(SyncStrategy strategy) {
    sync_strategy = strategy;
}

Repo::SyncStrategy Repo::get_sync_strategy() const noexcept {
    return sync_strategy;
}

//void Repo::download_url(const char * url, int fd) {
//    downloader->download_url(url, fd);
//}

void Repo::set_http_headers(const std::vector<std::string> & headers) {
    downloader->http_headers = headers;
}

std::vector<std::string> Repo::get_http_headers() const {
    return downloader->http_headers;
}

std::vector<std::string> Repo::get_mirrors() const {
    return downloader->mirrors;
}

BaseWeakPtr Repo::get_base() const {
    return base;
}


void Repo::make_solv_repo() {
    if (!solv_repo) {
        solv_repo.reset(new SolvRepo(base, config, this));

        // TODO(lukash) move the below to SolvRepo? Requires sharing Type
        if (type == Type::SYSTEM) {
            pool_set_installed(*get_rpm_pool(base), solv_repo->repo);
            pool_set_installed(*get_comps_pool(base), solv_repo->comps_repo);
        }

        solv_repo->set_priority(-get_priority());
        solv_repo->set_subpriority(-get_cost());
    }
}


void Repo::load_available_repo() {
    auto primary_fn = downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY);
    if (primary_fn.empty()) {
        throw RepoError(M_("Failed to load repository: \"primary\" data not present or in unsupported format"));
    }

    solv_repo->load_repo_main(downloader->repomd_filename, primary_fn);

    auto optional_metadata = config.get_main_config().get_optional_metadata_types_option().get_value();

    if (optional_metadata.contains(libdnf5::METADATA_TYPE_FILELISTS)) {
        solv_repo->load_repo_ext(RepodataType::FILELISTS, *downloader.get());
    }

    if (optional_metadata.contains(libdnf5::METADATA_TYPE_OTHER)) {
        solv_repo->load_repo_ext(RepodataType::OTHER, *downloader.get());
    }

    if (optional_metadata.contains(libdnf5::METADATA_TYPE_PRESTO)) {
        solv_repo->load_repo_ext(RepodataType::PRESTO, *downloader.get());
    }

    if (optional_metadata.contains(libdnf5::METADATA_TYPE_UPDATEINFO)) {
        solv_repo->load_repo_ext(RepodataType::UPDATEINFO, *downloader.get());
    }

    if (optional_metadata.contains(libdnf5::METADATA_TYPE_COMPS)) {
        solv_repo->load_repo_ext(RepodataType::COMPS, *downloader.get());
    }

    // Load module metadata
#ifdef MODULEMD
    auto & logger = *base->get_logger();

    std::string ext_fn = downloader->get_metadata_path(RepoDownloader::MD_FILENAME_MODULES);

    if (ext_fn.empty()) {
        logger.debug("No {} metadata available for repo {}", RepoDownloader::MD_FILENAME_MODULES, config.get_id());
        return;
    }

    logger.debug(
        "Loading {} extension for repo {} from \"{}\"", RepoDownloader::MD_FILENAME_MODULES, config.get_id(), ext_fn);
    auto yaml_content = libdnf5::utils::fs::File(ext_fn, "r", true).read();
    base->get_module_sack()->add(yaml_content, config.get_id());
#endif
}


// TODO(jkolarik): currently all metadata are loaded for system repo, maybe we want to have more control about that
void Repo::load_system_repo() {
    solv_repo->load_system_repo();
    solv_repo->load_system_repo_ext(RepodataType::COMPS);
}


rpm::Package Repo::add_rpm_package(const std::string & path, bool with_hdrid) {
    is_readable_rpm(path);

    make_solv_repo();

    int flags = REPO_REUSE_REPODATA | REPO_NO_INTERNALIZE;
    if (with_hdrid) {
        flags |= RPM_ADD_WITH_HDRID | RPM_ADD_WITH_SHA256SUM;
    }

    Id new_id = repo_add_rpm(solv_repo->repo, path.c_str(), flags);
    if (new_id == 0) {
        throw RepoRpmError(M_("Failed to load RPM \"{}\": {}"), path, std::string(pool_errstr(solv_repo->repo->pool)));
    }

    solv_repo->set_needs_internalizing();
    base->get_rpm_package_sack()->p_impl->invalidate_provides();

    return rpm::Package(base, rpm::PackageId(new_id));
}


void Repo::internalize() {
    if (solv_repo) {
        solv_repo->internalize();
    }
}


bool Repo::clone_root_metadata() {
    auto & logger = *base->get_logger();

    auto repo_cachedir = config.get_cachedir();
    auto base_cachedir = config.get_basecachedir_option().get_value();

    auto base_path_pos = repo_cachedir.find(base_cachedir);
    libdnf_assert(
        base_path_pos != std::string::npos,
        "Repo cachedir \"{}\" doesn't contain base cachedir \"{}\"",
        repo_cachedir,
        base_cachedir);

    auto root_repo_cachedir = repo_cachedir;
    root_repo_cachedir.replace(
        base_path_pos, base_cachedir.size(), config.get_main_config().get_system_cachedir_option().get_value());

    auto root_repodata_cachedir = std::filesystem::path(root_repo_cachedir) / CACHE_METADATA_DIR;
    try {
        if (!std::filesystem::exists(root_repodata_cachedir)) {
            return false;
        }
    } catch (const std::filesystem::filesystem_error & e) {
        logger.debug("Error when checking root repodata at \"{}\" : \"{}\"", root_repodata_cachedir.c_str(), e.what());
        return false;
    }

    auto repo_cache = RepoCache(base, repo_cachedir);
    repo_cache.remove_metadata();
    repo_cache.remove_solv_files();

    auto repodata_cachedir = std::filesystem::path(repo_cachedir) / CACHE_METADATA_DIR;
    try {
        std::filesystem::create_directories(repodata_cachedir);
        std::filesystem::copy(root_repodata_cachedir, repodata_cachedir);
    } catch (const std::filesystem::filesystem_error & e) {
        logger.debug(
            "Error when cloning root repodata from \"{}\" to \"{}\" : \"{}\"",
            root_repodata_cachedir.c_str(),
            repodata_cachedir.c_str(),
            e.what());
        repo_cache.remove_metadata();
        return false;
    }

    auto root_solv_cachedir = std::filesystem::path(root_repo_cachedir) / CACHE_SOLV_FILES_DIR;
    auto solv_cachedir = std::filesystem::path(repo_cachedir) / CACHE_SOLV_FILES_DIR;
    try {
        if (std::filesystem::exists(root_solv_cachedir)) {
            std::filesystem::create_directories(solv_cachedir);
            std::filesystem::copy(root_solv_cachedir, solv_cachedir);
        }
    } catch (const std::filesystem::filesystem_error & e) {
        logger.debug(
            "Error when cloning root solv data from \"{}\" to \"{}\" : \"{}\"",
            root_solv_cachedir.c_str(),
            solv_cachedir.c_str(),
            e.what());
        repo_cache.remove_solv_files();
    }

    return true;
}


void Repo::recompute_expired() {
    if (expired) {
        return;
    }

    if (RepoCache(base, get_cachedir()).is_attribute(RepoCache::ATTRIBUTE_EXPIRED)) {
        expired = true;
        return;
    }

    if (config.get_metadata_expire_option().get_value() == -1) {
        return;
    }

    if (config.get_main_config().get_check_config_file_age_option().get_value() && !repo_file_path.empty() &&
        mtime(repo_file_path.c_str()) >
            mtime(downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str())) {
        expired = true;
    } else {
        expired = get_age() > config.get_metadata_expire_option().get_value();
    }
}

std::string Repo::type_to_string(Type type) {
    switch (type) {
        case Type::AVAILABLE:
            return "available";
        case Type::COMMANDLINE:
            return "commandline";
        case Type::SYSTEM:
            return "system";
    }
    return "";
}

}  // namespace libdnf5::repo
