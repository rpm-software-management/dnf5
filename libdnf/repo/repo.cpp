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
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/fs/file.hpp"
#include "utils/string.hpp"

#include "libdnf/common/exception.hpp"
#include "libdnf/logger/logger.hpp"

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


namespace libdnf::repo {

static void is_readable_rpm(const std::string & fn) {
    if (std::filesystem::path(fn).extension() != ".rpm") {
        throw RepoRpmError(M_("Failed to load RPM \"{}\": doesn't have the \".rpm\" extension"), fn);
    }

    if (access(fn.c_str(), R_OK) != 0) {
        throw RepoRpmError(M_("Failed to access RPM \"{}\": {}"), fn, strerror(errno));
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
      downloader(new RepoDownloader(base, config)) {
    if (type == Type::AVAILABLE) {
        auto idx = verify_id(id);
        if (idx != std::string::npos) {
            throw RepoError(M_("Invalid repository id \"{}\": unexpected character '{}'"), id, id[idx]);
        }
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

std::string::size_type Repo::verify_id(const std::string & repo_id) {
    return repo_id.find_first_not_of(REPOID_CHARS);
}

void Repo::verify() const {
    if (config.baseurl().empty() && (config.metalink().empty() || config.metalink().get_value().empty()) &&
        (config.mirrorlist().empty() || config.mirrorlist().get_value().empty())) {
        throw RepoError(M_("Repository \"{}\" has no source (baseurl, mirrorlist or metalink) set."), config.get_id());
    }

    const auto & type = config.type().get_value();
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

std::string Repo::get_id() const noexcept {
    return config.get_id();
}

void Repo::enable() {
    config.enabled().set(Option::Priority::RUNTIME, true);
}

void Repo::disable() {
    config.enabled().set(Option::Priority::RUNTIME, false);
}

bool Repo::is_enabled() const {
    return config.enabled().get_value();
}

bool Repo::is_local() const {
    if ((!config.metalink().empty() && !config.metalink().get_value().empty()) ||
        (!config.mirrorlist().empty() && !config.mirrorlist().get_value().empty())) {
        return false;
    }
    if (!config.baseurl().get_value().empty() && config.baseurl().get_value()[0].compare(0, 7, "file://") == 0) {
        return true;
    }
    return false;
}

bool Repo::fetch_metadata() {
    auto & logger = *base->get_logger();

    if (downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).empty()) {
        // cache hasn't been loaded yet, try to load it
        try {
            read_metadata_cache();
        } catch (std::runtime_error & e) {
            // TODO(lukash) ideally we'd log something here, but we don't want it to look like an error
        }
    }

    if (!downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).empty()) {
        // cache loaded
        recompute_expired();
        if (!expired || sync_strategy == SyncStrategy::ONLY_CACHE || sync_strategy == SyncStrategy::LAZY) {
            logger.debug("Using cache for repo \"{}\"", config.get_id());
            return false;
        }

        if (is_in_sync()) {
            // the expired metadata still reflect the origin:
            utimes(downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str(), nullptr);
            RepoCache(base, config.get_cachedir()).remove_attribute(RepoCache::ATTRIBUTE_EXPIRED);
            expired = false;
            return true;
        }
    }
    if (sync_strategy == SyncStrategy::ONLY_CACHE) {
        throw RepoError(M_("Cache-only enabled but no cache for repository \"{}\""), config.get_id());
    }

    logger.debug("Downloading metadata for repo \"{}\"", config.get_id());
    auto cache_dir = config.get_cachedir();
    downloader->download_metadata(cache_dir);
    RepoCache(base, config.get_cachedir()).remove_attribute(RepoCache::ATTRIBUTE_EXPIRED);
    timestamp = -1;
    read_metadata_cache();

    expired = false;
    return true;
}

void Repo::read_metadata_cache() {
    downloader->load_local();

    // set timestamp unless explicitly expired
    if (timestamp != 0) {
        timestamp = mtime(downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str());
    }
}


bool Repo::is_in_sync() {
    if (!config.metalink().empty() && !config.metalink().get_value().empty()) {
        return downloader->is_metalink_in_sync();
    }
    return downloader->is_repomd_in_sync();
}


void Repo::download_metadata(const std::string & destdir) {
    downloader->download_metadata(destdir);
}

void Repo::load(LoadFlags flags) {
    make_solv_repo();

    if (type == Type::AVAILABLE) {
        load_available_repo(flags);
    } else if (type == Type::SYSTEM) {
        solv_repo->load_system_repo();
    }

    solv_repo->set_needs_internalizing();
    base->get_rpm_package_sack()->p_impl->invalidate_provides();
}

void Repo::load_extra_system_repo(const std::string & rootdir) {
    libdnf_assert(type == Type::SYSTEM, "repo type must be SYSTEM to load an extra system repo");
    libdnf_assert(solv_repo, "repo must be loaded to load an extra system repo");
    solv_repo->load_system_repo(rootdir);
}

void Repo::add_libsolv_testcase(const std::string & path) {
    make_solv_repo();

    libdnf::utils::fs::File testcase_file(path, "r", true);
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

void Repo::set_load_flags(LoadFlags value) {
    downloader->set_load_flags(value);
}

int Repo::get_cost() const {
    return config.cost().get_value();
}

void Repo::set_cost(int value, Option::Priority priority) {
    auto & conf_cost = config.cost();
    conf_cost.set(priority, value);
    if (solv_repo) {
        solv_repo->set_subpriority(-conf_cost.get_value());
    }
}

int Repo::get_priority() const {
    return config.priority().get_value();
}

void Repo::set_priority(int value, Option::Priority priority) {
    auto & conf_priority = config.priority();
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
    timestamp = 0;
}

bool Repo::is_expired() const {
    if (expired)
        // explicitly requested expired state
        return true;
    if (config.metadata_expire().get_value() == -1)
        return false;
    return get_age() > config.metadata_expire().get_value();
}

int Repo::get_expires_in() const {
    return config.metadata_expire().get_value() - static_cast<int>(get_age());
}

void Repo::set_substitutions(const std::map<std::string, std::string> & substitutions) {
    downloader->substitutions = substitutions;
}

void Repo::add_metadata_type_to_download(const std::string & metadata_type) {
    downloader->additional_metadata.insert(metadata_type);
}

void Repo::remove_metadata_type_from_download(const std::string & metadata_type) {
    downloader->additional_metadata.erase(metadata_type);
}

std::string Repo::get_metadata_path(const std::string & metadata_type) {
    return downloader->get_metadata_path(metadata_type);
}

bool Repo::fresh() {
    return timestamp >= 0;
}

void Repo::set_max_mirror_tries(int max_mirror_tries) {
    downloader->max_mirror_tries = max_mirror_tries;
}

int64_t Repo::get_timestamp() const {
    return timestamp;
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
            pool_set_installed(*get_pool(base), solv_repo->repo);
        }

        solv_repo->set_priority(-get_priority());
        solv_repo->set_subpriority(-get_cost());
    }
}


void Repo::load_available_repo(LoadFlags flags) {
    auto primary_fn = downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY);
    if (primary_fn.empty()) {
        throw RepoError(M_("Failed to load repository: \"primary\" data not present or in unsupported format"));
    }

    solv_repo->load_repo_main(downloader->repomd_filename, primary_fn);

    if (any(flags & LoadFlags::FILELISTS)) {
        solv_repo->load_repo_ext(RepodataType::FILELISTS, *downloader.get());
    }

    if (any(flags & LoadFlags::OTHER)) {
        solv_repo->load_repo_ext(RepodataType::OTHER, *downloader.get());
    }

    if (any(flags & LoadFlags::PRESTO)) {
        solv_repo->load_repo_ext(RepodataType::PRESTO, *downloader.get());
    }

    if (any(flags & LoadFlags::UPDATEINFO)) {
        solv_repo->load_repo_ext(RepodataType::UPDATEINFO, *downloader.get());
    }

    if (any(flags & LoadFlags::COMPS)) {
        solv_repo->load_repo_ext(RepodataType::COMPS, *downloader.get());
    }
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
        throw RepoRpmError(M_("Failed to load RPM \"{}\": {}"), path, pool_errstr(solv_repo->repo->pool));
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


void Repo::recompute_expired() {
    if (expired) {
        return;
    }

    if (RepoCache(base, get_cachedir()).is_attribute(RepoCache::ATTRIBUTE_EXPIRED)) {
        expired = true;
        return;
    }

    if (config.metadata_expire().get_value() == -1) {
        return;
    }

    if (config.get_main_config().check_config_file_age().get_value() && !repo_file_path.empty() &&
        mtime(repo_file_path.c_str()) >
            mtime(downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str())) {
        expired = true;
    } else {
        expired = get_age() > config.metadata_expire().get_value();
    }
}


// ============ librepo logging ===========

#define LR_LOGDOMAIN "librepo"

class LrHandleLogData {
public:
    std::string file_path;
    long uid;
    FILE * fd;
    bool used{false};
    guint handler_id;

    ~LrHandleLogData();
};

LrHandleLogData::~LrHandleLogData() {
    if (used)
        g_log_remove_handler(LR_LOGDOMAIN, handler_id);
    fclose(fd);
}

static std::list<std::unique_ptr<LrHandleLogData>> lr_log_datas;
static std::mutex lr_log_datas_mutex;

static const char * lr_log_level_flag_to_cstr(GLogLevelFlags log_level_flag) {
    if (log_level_flag & G_LOG_LEVEL_ERROR)
        return "ERROR";
    if (log_level_flag & G_LOG_LEVEL_CRITICAL)
        return "CRITICAL";
    if (log_level_flag & G_LOG_LEVEL_WARNING)
        return "WARNING";
    if (log_level_flag & G_LOG_LEVEL_MESSAGE)
        return "MESSAGE";
    if (log_level_flag & G_LOG_LEVEL_INFO)
        return "INFO";
    if (log_level_flag & G_LOG_LEVEL_DEBUG)
        return "DEBUG";
    return "USER";
}

static void librepo_log_cb(
    G_GNUC_UNUSED const gchar * log_domain, GLogLevelFlags log_level, const char * msg, gpointer user_data) noexcept {
    // Ignore exception during logging. Eg. exception generated during logging of exception is not good.
    try {
        auto data = static_cast<LrHandleLogData *>(user_data);
        auto now = time(nullptr);
        struct tm now_tm;
        gmtime_r(&now, &now_tm);

        std::ostringstream ss;
        ss << std::put_time(&now_tm, "%FT%TZ ");  // "YYYY-MM-DDTHH:MM:SSZ "
        ss << lr_log_level_flag_to_cstr(log_level) << " " << msg << std::endl;
        auto str = ss.str();
        fwrite(str.c_str(), sizeof(char), str.length(), data->fd);
        fflush(data->fd);
    } catch (const std::exception &) {
    }
}

long LibrepoLog::add_handler(const std::string & file_path, bool debug) {
    static long uid = 0;

    // Open the file
    FILE * fd = fopen(file_path.c_str(), "ae");
    if (!fd) {
        throw RuntimeError(M_("Cannot open {}: {}"), file_path, g_strerror(errno));
    }

    // Setup user data
    std::unique_ptr<LrHandleLogData> data(new LrHandleLogData);
    data->file_path = file_path;
    data->fd = fd;

    // Set handler
    GLogLevelFlags log_mask = debug ? G_LOG_LEVEL_MASK
                                    : static_cast<GLogLevelFlags>(
                                          G_LOG_LEVEL_INFO | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING |
                                          G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR);

    data->handler_id = g_log_set_handler(LR_LOGDOMAIN, log_mask, librepo_log_cb, data.get());
    data->used = true;

    // Save user data (in a thread safe way)
    {
        std::lock_guard<std::mutex> guard(lr_log_datas_mutex);

        // Get unique ID of the handler
        data->uid = ++uid;

        // Append the data to the global list
        lr_log_datas.push_front(std::move(data));
    }

    // Log librepo version and current time (including timezone)
    lr_log_librepo_summary();

    // Return unique id of the handler data
    return uid;
}

void LibrepoLog::remove_handler(long uid) {
    std::lock_guard<std::mutex> guard(lr_log_datas_mutex);

    // Search for the corresponding LogFileData
    auto it = lr_log_datas.begin();
    while (it != lr_log_datas.end() && (*it)->uid != uid) {
        ++it;
    }
    if (it == lr_log_datas.end()) {
        throw RuntimeError(M_("Log handler with id {} doesn't exist"), uid);
    }

    // Remove the handler and free the data
    lr_log_datas.erase(it);
}

void LibrepoLog::remove_all_handlers() {
    std::lock_guard<std::mutex> guard(lr_log_datas_mutex);
    lr_log_datas.clear();
}

}  // namespace libdnf::repo
