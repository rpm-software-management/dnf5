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

#include "repo_impl.hpp"
#include "rpm/package_sack_impl.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/string.hpp"

#include "libdnf/common/exception.hpp"
#include "libdnf/logger/logger.hpp"

extern "C" {
#include <solv/repo_rpmdb.h>
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


bool Repo::Impl::ends_with(const std::string & str, const std::string & ending) {
    if (str.length() >= ending.length()) {
        return (str.compare(str.length() - ending.length(), ending.length(), ending) == 0);
    }
    return false;
}

const std::string & Repo::Impl::get_metadata_path(const std::string & metadata_type) const {
    static const std::string empty;
    std::string lookup_metadata_type = metadata_type;
    if (config.get_main_config().zchunk().get_value()) {
        if (!ends_with(metadata_type, "_zck")) {
            lookup_metadata_type = metadata_type + "_zck";
        }
    }
    auto it = metadata_paths.find(lookup_metadata_type);
    if (it == metadata_paths.end() && lookup_metadata_type != metadata_type) {
        it = metadata_paths.find(metadata_type);
    }
    auto & ret = (it != metadata_paths.end()) ? it->second : empty;
    return ret;
}


Repo::Impl::Impl(const BaseWeakPtr & base, Repo & owner, std::string id, Type type)
    : type(type),
      config(base->get_config(), id),
      timestamp(-1),
      sync_strategy(SyncStrategy::TRY_CACHE),
      owner(&owner),
      base(base),
      expired(false),
      solv_repo(base, config),
      downloader(base, config) {}

Repo::Impl::~Impl() {
    if (solv_repo.repo) {
        solv_repo.repo->appdata = nullptr;
    }
}


void Repo::Impl::load_available_repo(LoadFlags flags) {
    auto & logger = *base->get_logger();

    auto primary_fn = get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY);
    if (primary_fn.empty()) {
        throw RepoError(_("Failed to load repository: \"primary\" data not present or in unsupported format"));
    }

    solv_repo.load_repo_main(downloader.get_repomd_filename(), primary_fn);

    if (any(flags & LoadFlags::FILELISTS)) {
        auto md_filename = get_metadata_path(RepoDownloader::MD_FILENAME_FILELISTS);

        if (!md_filename.empty()) {
            solv_repo.load_repo_ext(md_filename, RepodataType::FILELISTS);
        } else {
            logger.debug("No filelists metadata available for repo \"{}\"", config.get_id());
        }
    }
    if (any(flags & LoadFlags::OTHER)) {
        auto md_filename = get_metadata_path(RepoDownloader::MD_FILENAME_OTHER);

        if (!md_filename.empty()) {
            solv_repo.load_repo_ext(md_filename, RepodataType::OTHER);
        } else {
            logger.debug("No other metadata available for repo \"{}\"", config.get_id());
        }
    }
    if (any(flags & LoadFlags::PRESTO)) {
        auto md_filename = get_metadata_path(RepoDownloader::MD_FILENAME_PRESTODELTA);

        if (!md_filename.empty()) {
            solv_repo.load_repo_ext(md_filename, RepodataType::PRESTO);
        } else {
            logger.debug("No presto metadata available for repo \"{}\"", config.get_id());
        }
    }

    // updateinfo must come *after* all other extensions, as it is not a real
    //   extension, but contains a new set of packages
    if (any(flags & LoadFlags::UPDATEINFO)) {
        auto md_filename = get_metadata_path(RepoDownloader::MD_FILENAME_UPDATEINFO);

        if (!md_filename.empty()) {
            solv_repo.load_repo_ext(md_filename, RepodataType::UPDATEINFO);
        } else {
            logger.debug("No updateinfo metadata available for repo \"{}\"", config.get_id());
        }
    }

    if (any(flags & LoadFlags::COMPS)) {
        auto md_filename = get_metadata_path(RepoDownloader::MD_FILENAME_GROUP_GZ);
        if (md_filename.empty()) {
            md_filename = get_metadata_path(RepoDownloader::MD_FILENAME_GROUP);
        }

        if (!md_filename.empty()) {
            solv_repo.load_repo_ext(md_filename, RepodataType::COMPS);
        } else {
            logger.debug("No group metadata available for repo \"{}\"", config.get_id());
        }
    }

    base->get_rpm_package_sack()->p_impl->invalidate_provides();
}

Repo::Repo(const BaseWeakPtr & base, const std::string & id, Repo::Type type) {
    if (type == Type::AVAILABLE) {
        auto idx = verify_id(id);
        if (idx != std::string::npos) {
            throw RepoError(M_("Invalid repository id \"{}\": unexpected character '{}'"), id, id[idx]);
        }
    }
    p_impl.reset(new Impl(base, *this, id, type));

    // TODO(lukash) move this to SolvRepo constructor
    p_impl->solv_repo.repo->appdata = this;
    p_impl->solv_repo.repo->subpriority = -get_cost();
    p_impl->solv_repo.repo->priority = -get_priority();
}

Repo::Repo(Base & base, const std::string & id, Repo::Type type) : Repo(base.get_weak_ptr(), id, type) {}

Repo::~Repo() = default;

Repo::Type Repo::get_type() const noexcept {
    return p_impl->type;
}

void Repo::set_callbacks(std::unique_ptr<RepoCallbacks> && callbacks) {
    p_impl->downloader.set_callbacks(std::move(callbacks));
}

std::string::size_type Repo::verify_id(const std::string & repo_id) {
    return repo_id.find_first_not_of(REPOID_CHARS);
}

void Repo::verify() const {
    if (p_impl->config.baseurl().empty() &&
        (p_impl->config.metalink().empty() || p_impl->config.metalink().get_value().empty()) &&
        (p_impl->config.mirrorlist().empty() || p_impl->config.mirrorlist().get_value().empty())) {
        throw RepoError(
            M_("Repository \"{}\" has no source (baseurl, mirrorlist or metalink) set."), p_impl->config.get_id());
    }

    const auto & type = p_impl->config.type().get_value();
    const char * supported_repo_types[]{"rpm-md", "rpm", "repomd", "rpmmd", "yum", "YUM"};
    if (!type.empty()) {
        for (auto supported : supported_repo_types) {
            if (type == supported) {
                return;
            }
        }
        throw RepoError(M_("Repository \"{}\" has unsupported type \"{}\", skipping."), p_impl->config.get_id(), type);
    }
}

ConfigRepo & Repo::get_config() noexcept {
    return p_impl->config;
}

std::string Repo::get_id() const noexcept {
    return p_impl->config.get_id();
}

void Repo::enable() {
    p_impl->config.enabled().set(Option::Priority::RUNTIME, true);
}

void Repo::disable() {
    p_impl->config.enabled().set(Option::Priority::RUNTIME, false);
}

bool Repo::is_enabled() const {
    return p_impl->config.enabled().get_value();
}

bool Repo::is_local() const {
    auto & config = p_impl->config;
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
    return p_impl->fetch_metadata();
}

void Repo::read_metadata_cache() {
    p_impl->read_metadata_cache();
}

void Repo::download_metadata(const std::string & destdir) {
    p_impl->downloader.download_metadata(destdir);
}

void Repo::load(LoadFlags flags) {
    if (p_impl->type == Type::AVAILABLE) {
        p_impl->load_available_repo(flags);
    } else if (p_impl->type == Type::SYSTEM) {
        p_impl->solv_repo.load_system_repo();
    }
}

void Repo::load_extra_system_repo(const std::string & rootdir) {
    libdnf_assert(p_impl->type == Type::SYSTEM, "repo type must be SYSTEM to load an extra system repo");
    p_impl->solv_repo.load_system_repo(rootdir);
}

bool Repo::get_use_includes() const {
    return p_impl->use_includes;
}

void Repo::set_use_includes(bool enabled) {
    p_impl->use_includes = enabled;
}

bool Repo::get_load_metadata_other() const {
    return p_impl->downloader.load_metadata_other;
}

void Repo::set_load_metadata_other(bool value) {
    p_impl->downloader.load_metadata_other = value;
}

int Repo::get_cost() const {
    return p_impl->config.cost().get_value();
}

void Repo::set_cost(int value, Option::Priority priority) {
    auto & conf_cost = p_impl->config.cost();
    conf_cost.set(priority, value);
    if (p_impl->solv_repo.repo != nullptr) {
        p_impl->solv_repo.repo->subpriority = -conf_cost.get_value();
    }
}

int Repo::get_priority() const {
    return p_impl->config.priority().get_value();
}

void Repo::set_priority(int value, Option::Priority priority) {
    auto & conf_priority = p_impl->config.priority();
    conf_priority.set(priority, value);
    if (p_impl->solv_repo.repo != nullptr) {
        p_impl->solv_repo.repo->priority = -conf_priority.get_value();
    }
}

int64_t Repo::get_age() const {
    return p_impl->get_age();
}

void Repo::expire() {
    p_impl->expire();
}

bool Repo::is_expired() const {
    return p_impl->is_expired();
}

int Repo::get_expires_in() const {
    return p_impl->get_expires_in();
}

void Repo::set_substitutions(const std::map<std::string, std::string> & substitutions) {
    p_impl->downloader.substitutions = substitutions;
}

void Repo::add_metadata_type_to_download(const std::string & metadata_type) {
    p_impl->downloader.additional_metadata.insert(metadata_type);
}

void Repo::remove_metadata_type_from_download(const std::string & metadata_type) {
    p_impl->downloader.additional_metadata.erase(metadata_type);
}

std::string Repo::get_metadata_path(const std::string & metadata_type) {
    return p_impl->get_metadata_path(metadata_type);
}


void Repo::Impl::read_metadata_cache() {
    downloader.load_local();

    revision = downloader.get_revision();
    max_timestamp = downloader.get_max_timestamp();
    metadata_paths = downloader.get_metadata_paths();
    content_tags = downloader.get_content_tags();
    distro_tags = downloader.get_distro_tags();
    metadata_locations = downloader.get_metadata_locations();

    // Load timestamp unless explicitly expired
    if (timestamp != 0) {
        timestamp = mtime(get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str());
    }
}

bool Repo::Impl::try_load_cache() {
    try {
        read_metadata_cache();
    } catch (std::runtime_error &) {
        return false;
    }
    return true;
}

bool Repo::Impl::is_in_sync() {
    if (!config.metalink().empty() && !config.metalink().get_value().empty()) {
        return downloader.is_metalink_in_sync();
    }
    return downloader.is_repomd_in_sync();
}

bool Repo::Impl::fetch_metadata() {
    auto & logger = *base->get_logger();

    if (!get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).empty() || try_load_cache()) {
        reset_metadata_expired();
        if (!expired || sync_strategy == SyncStrategy::ONLY_CACHE || sync_strategy == SyncStrategy::LAZY) {
            logger.debug("Using cache for repo \"{}\"", config.get_id());
            return false;
        }

        if (is_in_sync()) {
            // the expired metadata still reflect the origin:
            utimes(get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str(), nullptr);
            expired = false;
            return true;
        }
    }
    if (sync_strategy == SyncStrategy::ONLY_CACHE) {
        throw RepoError(M_("Cache-only enabled but no cache for repository \"{}\""), config.get_id());
    }

    logger.debug("Downloading metadata for repo \"{}\"", config.get_id());
    downloader.download_metadata(config.get_cachedir());
    timestamp = -1;
    read_metadata_cache();

    expired = false;
    return true;
}

int64_t Repo::Impl::get_age() const {
    return time(nullptr) - mtime(get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str());
}

void Repo::Impl::expire() {
    expired = true;
    timestamp = 0;
}

bool Repo::Impl::is_expired() const {
    if (expired)
        // explicitly requested expired state
        return true;
    if (config.metadata_expire().get_value() == -1)
        return false;
    return get_age() > config.metadata_expire().get_value();
}

int Repo::Impl::get_expires_in() const {
    return config.metadata_expire().get_value() - static_cast<int>(get_age());
}

bool Repo::fresh() {
    return p_impl->timestamp >= 0;
}

void Repo::Impl::reset_metadata_expired() {
    if (expired || config.metadata_expire().get_value() == -1)
        return;
    if (config.get_main_config().check_config_file_age().get_value() && !repo_file_path.empty() &&
        mtime(repo_file_path.c_str()) > mtime(get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str()))
        expired = true;
    else
        expired = get_age() > config.metadata_expire().get_value();
}

void Repo::set_max_mirror_tries(int max_mirror_tries) {
    p_impl->downloader.max_mirror_tries = max_mirror_tries;
}

int64_t Repo::get_timestamp() const {
    return p_impl->timestamp;
}

int Repo::get_max_timestamp() {
    return p_impl->max_timestamp;
}

void Repo::set_preserve_remote_time(bool preserve_remote_time) {
    p_impl->downloader.preserve_remote_time = preserve_remote_time;
}

bool Repo::get_preserve_remote_time() const {
    return p_impl->downloader.preserve_remote_time;
}

const std::vector<std::string> & Repo::get_content_tags() {
    return p_impl->content_tags;
}

const std::vector<std::pair<std::string, std::string>> & Repo::get_distro_tags() {
    return p_impl->distro_tags;
}

const std::vector<std::pair<std::string, std::string>> Repo::get_metadata_locations() const {
    return p_impl->metadata_locations;
}

std::string Repo::get_cachedir() const {
    return p_impl->config.get_cachedir();
}

std::string Repo::get_persistdir() const {
    return p_impl->config.get_persistdir();
}

const std::string & Repo::get_revision() const {
    return p_impl->revision;
}

void Repo::set_repo_file_path(const std::string & path) {
    p_impl->repo_file_path = path;
}

const std::string & Repo::get_repo_file_path() const noexcept {
    return p_impl->repo_file_path;
}

void Repo::set_sync_strategy(SyncStrategy strategy) {
    p_impl->sync_strategy = strategy;
}

Repo::SyncStrategy Repo::get_sync_strategy() const noexcept {
    return p_impl->sync_strategy;
}

//void Repo::download_url(const char * url, int fd) {
//    p_impl->downloader.download_url(url, fd);
//}

void Repo::set_http_headers(const std::vector<std::string> & headers) {
    p_impl->downloader.http_headers = headers;
}

std::vector<std::string> Repo::get_http_headers() const {
    return p_impl->downloader.http_headers;
}

std::vector<std::string> Repo::get_mirrors() const {
    return p_impl->downloader.get_mirrors();
}

BaseWeakPtr Repo::get_base() const {
    return p_impl->base;
}

Id Repo::Impl::add_rpm_package(const std::string & fn, bool add_with_hdrid) {
    is_readable_rpm(fn);

    int flags = REPO_REUSE_REPODATA | REPO_NO_INTERNALIZE;
    if (add_with_hdrid) {
        flags |= RPM_ADD_WITH_HDRID | RPM_ADD_WITH_SHA256SUM;
    }

    Id new_id = repo_add_rpm(solv_repo.repo, fn.c_str(), flags);
    if (new_id == 0) {
        throw RepoRpmError(M_("Failed to load RPM \"{}\": {}"), fn, pool_errstr(solv_repo.repo->pool));
    }
    solv_repo.set_needs_internalizing();
    return new_id;
}

// TODO(jrohel): Later with rpm sack work.
/* void repo_internalize_trigger(LibsolvRepo * repo) {
    if (!repo)
        return;

    if (auto hrepo = static_cast<libdnf::Repo *>(repo->appdata)) {
        // HyRepo is attached. The hint needs_internalizing will be used.
        auto repoImpl = libdnf::repo_get_impl(hrepo);
        assert(repoImpl->libsolv_repo == repo);
        if (!repoImpl->needs_internalizing)
            return;
        repoImpl->needs_internalizing = false;
    }

    repo_internalize(repo);
}

void repo_internalize_all_trigger(Pool * pool) {
    int i;
    LibsolvRepo * repo;

    FOR_REPOS(i, repo)
    repo_internalize_trigger(repo);
}
*/

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
        throw RuntimeError(fmt::format(M_("Cannot open {}: {}"), file_path, g_strerror(errno)));
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
        throw RuntimeError(fmt::format(M_("Log handler with id {} doesn't exist"), uid));
    }

    // Remove the handler and free the data
    lr_log_datas.erase(it);
}

void LibrepoLog::remove_all_handlers() {
    std::lock_guard<std::mutex> guard(lr_log_datas_mutex);
    lr_log_datas.clear();
}

}  // namespace libdnf::repo
