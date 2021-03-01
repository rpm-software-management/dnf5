/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#define METADATA_RELATIVE_DIR "repodata"
#define PACKAGES_RELATIVE_DIR "packages"
#define METALINK_FILENAME     "metalink.xml"
#define MIRRORLIST_FILENAME   "mirrorlist"
#define RECOGNIZED_CHKSUMS \
    { "sha512", "sha256" }

#define USER_AGENT "libdnf"

constexpr const char * REPOID_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.:";

#include "../libdnf/utils/bgettext/bgettext-lib.h"
#include "repo_impl.hpp"

#include "libdnf/logger/logger.hpp"
#include "libdnf/utils/utils.hpp"

extern "C" {
#include <solv/repo_rpmdb.h>
}

#include <fcntl.h>
#include <fmt/format.h>
#include <glib.h>
#include <gpgme.h>
#include <librepo/librepo.h>
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>
#include <sys/stat.h>
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
#include <random>
#include <set>
#include <sstream>
#include <system_error>
#include <type_traits>

//
// COUNTME CONSTANTS
//
// width of the sliding time window (in seconds)
const int COUNTME_WINDOW = 7 * 24 * 60 * 60;  // 1 week
// starting point of the sliding time window relative to the UNIX epoch
// allows for aligning the window with a specific weekday
const int COUNTME_OFFSET = 345600;  // Monday (1970-01-05 00:00:00 UTC)
// estimated number of metalink requests sent over the window
// used to generate the probability distribution of counting events
const int COUNTME_BUDGET = 4;  // metadata_expire defaults to 2 days
// cookie file name
const std::string COUNTME_COOKIE = "countme";
// cookie file format version
const int COUNTME_VERSION = 0;
// longevity buckets that we report in the flag
// example: {A, B, C} defines 4 buckets [0, A), [A, B), [B, C), [C, infinity)
// where each letter represents a window step (starting from 0)
const std::array<const int, 3> COUNTME_BUCKETS = {{2, 5, 25}};

namespace libdnf {
namespace {

/// Throw libdnf::RuntimeError when problem detected
void is_readable_rpm(const char * fn) {
    if (access(fn, R_OK) != 0) {
        const char * err_txt = strerror(errno);
        throw RuntimeError(fmt::format(_("Failed to access RPM: \"{}\": {}"), fn, err_txt));
    }

    auto len = strlen(fn);

    if (len <= 4 || (strcmp(fn + len - 4, ".rpm") != 0)) {
        throw RuntimeError(fmt::format(_("Failed to read RPM: \"{}\": {}"), fn, "does't have extension \".rpm\""));
    }
}


}  // namespace
}  // namespace libdnf


namespace std {

template <>
struct default_delete<GError> {
    void operator()(GError * ptr) noexcept { g_error_free(ptr); }
};

template <>
struct default_delete<LrResult> {
    void operator()(LrResult * ptr) noexcept { lr_result_free(ptr); }
};

template <>
struct default_delete<LrPackageTarget> {
    void operator()(LrPackageTarget * ptr) noexcept { lr_packagetarget_free(ptr); }
};

template <>
struct default_delete<std::remove_pointer<gpgme_ctx_t>::type> {
    void operator()(gpgme_ctx_t ptr) noexcept { gpgme_release(ptr); }
};

}  // namespace std

namespace libdnf::rpm {

class LrExceptionWithSourceUrl : public LrException {
public:
    LrExceptionWithSourceUrl(int code, const std::string & msg, std::string source_url)
        : LrException(code, msg)
        , source_url(std::move(source_url)) {}
    const std::string & get_source_url() const { return source_url; }

private:
    std::string source_url;
};

/// Recursive renames/moves file/directory.
/// Implements copy and remove fallback.
static void move_recursive(const std::string & src, const std::string & dest) {
    try {
        std::filesystem::rename(src, dest);
    } catch (const std::filesystem::filesystem_error & ex) {
        std::filesystem::copy(src, dest, std::filesystem::copy_options::recursive);
        std::filesystem::remove_all(src);
    }
}

static int64_t mtime(const char * filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_mtime;
}

static void throw_exception(std::unique_ptr<GError> && err) {
    throw LrException(err->code, err->message);
}

template <typename T>
inline static void handle_set_opt(LrHandle * handle, LrHandleOption option, T value) {
    GError * err_p{nullptr};
    if (!lr_handle_setopt(handle, &err_p, option, value)) {
        throw_exception(std::unique_ptr<GError>(err_p));
    }
}

inline static void handle_get_info(LrHandle * handle, LrHandleInfoOption option, void * value) {
    GError * err_p{nullptr};
    if (!lr_handle_getinfo(handle, &err_p, option, value)) {
        throw_exception(std::unique_ptr<GError>(err_p));
    }
}

template <typename T>
inline static void result_get_info(LrResult * result, LrResultInfoOption option, T value) {
    GError * err_p{nullptr};
    if (!lr_result_getinfo(result, &err_p, option, value)) {
        throw_exception(std::unique_ptr<GError>(err_p));
    }
}


// Map string from config option proxy_auth_method to librepo LrAuth value
static constexpr struct {
    const char * name;
    LrAuth code;
} PROXYAUTHMETHODS[] = {
    {"none", LR_AUTH_NONE},
    {"basic", LR_AUTH_BASIC},
    {"digest", LR_AUTH_DIGEST},
    {"negotiate", LR_AUTH_NEGOTIATE},
    {"ntlm", LR_AUTH_NTLM},
    {"digest_ie", LR_AUTH_DIGEST_IE},
    {"ntlm_wb", LR_AUTH_NTLM_WB},
    {"any", LR_AUTH_ANY}};

static LrAuth string_to_proxy_auth_methods(const std::string & proxy_auth_method_str) noexcept {
    auto proxy_auth_methods = LR_AUTH_ANY;
    for (auto & auth : PROXYAUTHMETHODS) {
        if (proxy_auth_method_str == auth.name) {
            proxy_auth_methods = auth.code;
            break;
        }
    }
    return proxy_auth_methods;
}


// Callback stuff
void RepoCB::start([[maybe_unused]] const char * what) {}
int RepoCB::progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) {
    return 0;
}
void RepoCB::fastest_mirror([[maybe_unused]] FastestMirrorStage stage, [[maybe_unused]] const char * ptr) {}
int RepoCB::handle_mirror_failure(
    [[maybe_unused]] const char * msg, [[maybe_unused]] const char * url, [[maybe_unused]] const char * metadata) {
    return 0;
}

bool RepoCB::repokey_import(
    const std::string & /*id*/,
    const std::string & /*userId*/,
    const std::string & /*fingerprint*/,
    const std::string & /*url*/,
    long int /*timestamp*/) {
    return true;
}


bool Repo::Impl::ends_with(const std::string & str, const std::string & ending) {
    if (str.length() >= ending.length()) {
        return (str.compare(str.length() - ending.length(), ending.length(), ending) == 0);
    }
    return false;
}

const std::string & Repo::Impl::get_metadata_path(const std::string & metadata_type) const {
    auto & logger = base->get_logger();
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
    if (ret.empty()) {
        logger.debug(fmt::format("not found \"{}\" for: {}", metadata_type, id));
    }
    return ret;
}

int Repo::Impl::progress_cb(void * data, double total_to_download, double downloaded) {
    if (!data) {
        return 0;
    }
    auto cb_object = static_cast<RepoCB *>(data);
    return cb_object->progress(total_to_download, downloaded);
}

void Repo::Impl::fastest_mirror_cb(void * data, LrFastestMirrorStages stage, void * ptr) {
    if (!data) {
        return;
    }
    auto cb_object = static_cast<RepoCB *>(data);
    const char * msg;
    std::string msg_string;
    if (ptr) {
        switch (stage) {
            case LR_FMSTAGE_CACHELOADING:
            case LR_FMSTAGE_CACHELOADINGSTATUS:
            case LR_FMSTAGE_STATUS:
                msg = static_cast<const char *>(ptr);
                break;
            case LR_FMSTAGE_DETECTION:
                msg_string = std::to_string(*(static_cast<long *>(ptr)));
                msg = msg_string.c_str();
                break;
            default:
                msg = nullptr;
        }
    } else {
        msg = nullptr;
    }
    cb_object->fastest_mirror(static_cast<RepoCB::FastestMirrorStage>(stage), msg);
}

int Repo::Impl::mirror_failure_cb(void * data, const char * msg, const char * url, const char * metadata) {
    if (!data) {
        return 0;
    }
    auto cb_object = static_cast<RepoCB *>(data);
    return cb_object->handle_mirror_failure(msg, url, metadata);
};


/// Converts the given input string to a URL encoded string
/// All input characters that are not a-z, A-Z, 0-9, '-', '.', '_' or '~' are converted
/// to their "URL escaped" version (%NN where NN is a two-digit hexadecimal number).
/// @param src String to encode
/// @return URL encoded string
static std::string url_encode(const std::string & src) {
    auto no_encode = [](char ch) { return isalnum(ch) != 0 || ch == '-' || ch == '.' || ch == '_' || ch == '~'; };

    // compute length of encoded string
    auto len = src.length();
    for (auto ch : src) {
        if (!no_encode(ch)) {
            len += 2;
        }
    }

    // encode the input string
    std::string encoded;
    encoded.reserve(len);
    for (auto ch : src) {
        if (no_encode(ch)) {
            encoded.push_back(ch);
        } else {
            encoded.push_back('%');
            int hex;
            hex = static_cast<unsigned char>(ch) >> 4;
            hex += hex <= 9 ? '0' : 'a' - 10;
            encoded.push_back(static_cast<char>(hex));
            hex = static_cast<unsigned char>(ch) & 0x0F;
            hex += hex <= 9 ? '0' : 'a' - 10;
            encoded.push_back(static_cast<char>(hex));
        }
    }

    return encoded;
}

/// Format user password string
/// Returns user and password in user:password form. If quote is True,
/// special characters in user and password are URL encoded.
/// @param user Username
/// @param passwd Password
/// @param encode If quote is True, special characters in user and password are URL encoded.
/// @return User and password in user:password form
static std::string format_user_pass_string(const std::string & user, const std::string & passwd, bool encode) {
    if (encode) {
        return url_encode(user) + ":" + url_encode(passwd);
    } else {
        return user + ":" + passwd;
    }
}

Repo::Impl::Impl(Repo & owner, std::string id, Type type, Base & base)
    : id(std::move(id))
    , type(type)
    , config(base.get_config())
    , timestamp(-1)
    , load_metadata_other(false)
    , sync_strategy(SyncStrategy::TRY_CACHE)
    , owner(&owner)
    , base(&base)
    , expired(false) {}

Repo::Impl::~Impl() {
    g_strfreev(mirrors);
    if (libsolv_repo_ext.repo) {
        libsolv_repo_ext.repo->appdata = nullptr;
    }
}

Repo::Repo(const std::string & id, Base & base, Repo::Type type) {
    if (type == Type::AVAILABLE) {
        auto idx = verify_id(id);
        if (idx >= 0) {
            std::string msg = fmt::format(_("Bad id for repo: {}, byte = {} {}"), id, id[idx], idx);
            throw RuntimeError(msg);
        }
    }
    p_impl.reset(new Impl(*this, id, type, base));
}

Repo::~Repo() = default;

void Repo::set_callbacks(std::unique_ptr<RepoCB> && callbacks) {
    p_impl->callbacks = std::move(callbacks);
}

int Repo::verify_id(const std::string & repo_id) {
    auto idx = repo_id.find_first_not_of(REPOID_CHARS);
    return idx == repo_id.npos ? -1 : static_cast<int>(idx);
}

void Repo::verify() const {
    if (p_impl->config.baseurl().empty() &&
        (p_impl->config.metalink().empty() || p_impl->config.metalink().get_value().empty()) &&
        (p_impl->config.mirrorlist().empty() || p_impl->config.mirrorlist().get_value().empty()))
        throw RuntimeError(fmt::format(_("Repository {} has no mirror or baseurl set."), p_impl->id));

    const auto & type = p_impl->config.type().get_value();
    const char * supported_repo_types[]{"rpm-md", "rpm", "repomd", "rpmmd", "yum", "YUM"};
    if (!type.empty()) {
        for (auto supported : supported_repo_types) {
            if (type == supported) {
                return;
            }
        }
        throw RuntimeError(
            fmt::format(_("Repository '{}' has unsupported type: 'type={}', skipping."), p_impl->id, type));
    }
}

ConfigRepo & Repo::get_config() noexcept {
    return p_impl->config;
}

const std::string & Repo::get_id() const noexcept {
    return p_impl->id;
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

bool Repo::load() {
    return p_impl->load();
}
void Repo::load_cache() {
    p_impl->load_cache();
}
void Repo::download_metadata(const std::string & destdir) {
    p_impl->download_metadata(destdir);
}
bool Repo::get_use_includes() const {
    return p_impl->use_includes;
}
void Repo::set_use_includes(bool enabled) {
    p_impl->use_includes = enabled;
}
bool Repo::get_load_metadata_other() const {
    return p_impl->load_metadata_other;
}
void Repo::set_load_metadata_other(bool value) {
    p_impl->load_metadata_other = value;
}
int Repo::get_cost() const {
    return p_impl->config.cost().get_value();
}
void Repo::set_cost(int value, Option::Priority priority) {
    auto & conf_cost = p_impl->config.cost();
    conf_cost.set(priority, value);
    if (p_impl->libsolv_repo_ext.repo != nullptr) {
        p_impl->libsolv_repo_ext.repo->subpriority = -conf_cost.get_value();
    }
}
int Repo::get_priority() const {
    return p_impl->config.priority().get_value();
}
void Repo::set_priority(int value, Option::Priority priority) {
    auto & conf_priority = p_impl->config.priority();
    conf_priority.set(priority, value);
    if (p_impl->libsolv_repo_ext.repo != nullptr) {
        p_impl->libsolv_repo_ext.repo->priority = -conf_priority.get_value();
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
    p_impl->substitutions = substitutions;
}

void Repo::add_metadata_type_to_download(const std::string & metadata_type) {
    p_impl->additional_metadata.insert(metadata_type);
}

void Repo::remove_metadata_type_from_download(const std::string & metadata_type) {
    p_impl->additional_metadata.erase(metadata_type);
}

std::string Repo::get_metadata_path(const std::string & metadata_type) {
    return p_impl->get_metadata_path(metadata_type);
}

std::unique_ptr<LrHandle> Repo::Impl::lr_handle_init_base() {
    std::unique_ptr<LrHandle> h(lr_handle_init());
    std::vector<const char *> dlist = {
        MD_FILENAME_PRIMARY,
        MD_FILENAME_FILELISTS,
        MD_FILENAME_PRESTODELTA,
        MD_FILENAME_GROUP_GZ,
        MD_FILENAME_UPDATEINFO};

#ifdef MODULEMD
    dlist.push_back(MD_FILENAME_MODULES);
#endif
    if (load_metadata_other) {
        dlist.push_back(MD_FILENAME_OTHER);
    }
    for (auto & item : additional_metadata) {
        dlist.push_back(item.c_str());
    }
    dlist.push_back(nullptr);
    handle_set_opt(h.get(), LRO_PRESERVETIME, static_cast<long>(preserve_remote_time));
    handle_set_opt(h.get(), LRO_REPOTYPE, LR_YUMREPO);
    handle_set_opt(h.get(), LRO_USERAGENT, config.user_agent().get_value().c_str());
    handle_set_opt(h.get(), LRO_YUMDLIST, dlist.data());
    handle_set_opt(h.get(), LRO_INTERRUPTIBLE, 1L);
    handle_set_opt(h.get(), LRO_GPGCHECK, config.repo_gpgcheck().get_value());
    handle_set_opt(h.get(), LRO_MAXMIRRORTRIES, static_cast<long>(max_mirror_tries));
    handle_set_opt(h.get(), LRO_MAXPARALLELDOWNLOADS, config.max_parallel_downloads().get_value());

    LrUrlVars * vars = nullptr;
    vars = lr_urlvars_set(vars, MD_FILENAME_GROUP_GZ, MD_FILENAME_GROUP);
    handle_set_opt(h.get(), LRO_YUMSLIST, vars);

    return h;
}

std::unique_ptr<LrHandle> Repo::Impl::lr_handle_init_local() {
    std::unique_ptr<LrHandle> h(lr_handle_init_base());

    LrUrlVars * vars = nullptr;
    for (const auto & item : substitutions) {
        vars = lr_urlvars_set(vars, item.first.c_str(), item.second.c_str());
    }
    handle_set_opt(h.get(), LRO_VARSUB, vars);
    auto cachedir = get_cachedir();
    handle_set_opt(h.get(), LRO_DESTDIR, cachedir.c_str());
    const char * urls[] = {cachedir.c_str(), nullptr};
    handle_set_opt(h.get(), LRO_URLS, urls);
    handle_set_opt(h.get(), LRO_LOCAL, 1L);
#ifdef LRO_SUPPORTS_CACHEDIR
    /* If zchunk is enabled, set librepo cache dir */
    if (config.get_main_config().zchunk().get_value()) {
        handle_set_opt(h.get(), LRO_CACHEDIR, config.basecachedir().get_value().c_str());
    }
#endif
    return h;
}

std::unique_ptr<LrHandle> Repo::Impl::lr_handle_init_remote(const char * destdir, bool mirror_setup) {
    std::unique_ptr<LrHandle> h(lr_handle_init_base());
    handle_set_opt(h.get(), LRO_HTTPHEADER, http_headers.get());

    LrUrlVars * vars = nullptr;
    for (const auto & item : substitutions) {
        vars = lr_urlvars_set(vars, item.first.c_str(), item.second.c_str());
    }
    handle_set_opt(h.get(), LRO_VARSUB, vars);

    handle_set_opt(h.get(), LRO_DESTDIR, destdir);

    auto & ip_resolve = config.ip_resolve().get_value();
    if (ip_resolve == "ipv4") {
        handle_set_opt(h.get(), LRO_IPRESOLVE, LR_IPRESOLVE_V4);
    } else if (ip_resolve == "ipv6") {
        handle_set_opt(h.get(), LRO_IPRESOLVE, LR_IPRESOLVE_V6);
    }

    enum class Source { NONE, METALINK, MIRRORLIST } source{Source::NONE};
    std::string tmp;
    if (!config.metalink().empty() && !(tmp = config.metalink().get_value()).empty()) {
        source = Source::METALINK;
    } else if (!config.mirrorlist().empty() && !(tmp = config.mirrorlist().get_value()).empty()) {
        source = Source::MIRRORLIST;
    }
    if (source != Source::NONE) {
        handle_set_opt(h.get(), LRO_HMFCB, static_cast<LrHandleMirrorFailureCb>(mirror_failure_cb));
        handle_set_opt(h.get(), LRO_PROGRESSDATA, callbacks.get());
        if (mirror_setup) {
            if (source == Source::METALINK) {
                handle_set_opt(h.get(), LRO_METALINKURL, tmp.c_str());
            } else {
                handle_set_opt(h.get(), LRO_MIRRORLISTURL, tmp.c_str());
                // YUM-DNF compatibility hack. YUM guessed by content of keyword "metalink" if
                // mirrorlist is really mirrorlist or metalink)
                if (tmp.find("metalink") != tmp.npos)
                    handle_set_opt(h.get(), LRO_METALINKURL, tmp.c_str());
            }
            handle_set_opt(h.get(), LRO_FASTESTMIRROR, config.fastestmirror().get_value() ? 1L : 0L);
            auto fastest_mirror_cache_dir = config.basecachedir().get_value();
            if (fastest_mirror_cache_dir.back() != '/') {
                fastest_mirror_cache_dir.push_back('/');
            }
            fastest_mirror_cache_dir += "fastestmirror.cache";
            handle_set_opt(h.get(), LRO_FASTESTMIRRORCACHE, fastest_mirror_cache_dir.c_str());
        } else {
            // use already resolved mirror list
            handle_set_opt(h.get(), LRO_URLS, mirrors);
        }
    } else if (!config.baseurl().get_value().empty()) {
        handle_set_opt(h.get(), LRO_HMFCB, static_cast<LrHandleMirrorFailureCb>(mirror_failure_cb));
        size_t len = config.baseurl().get_value().size();
        const char * urls[len + 1];
        for (size_t idx = 0; idx < len; ++idx) {
            urls[idx] = config.baseurl().get_value()[idx].c_str();
        }
        urls[len] = nullptr;
        handle_set_opt(h.get(), LRO_URLS, urls);
    } else {
        throw RuntimeError(fmt::format(_("Cannot find a valid baseurl for repo: {}"), id));
    }

    // setup username/password if needed
    auto userpwd = config.username().get_value();
    if (!userpwd.empty()) {
        // TODO Use URL encoded form, needs support in librepo
        userpwd = format_user_pass_string(userpwd, config.password().get_value(), false);
        handle_set_opt(h.get(), LRO_USERPWD, userpwd.c_str());
    }

    // setup ssl stuff
    if (!config.sslcacert().get_value().empty()) {
        handle_set_opt(h.get(), LRO_SSLCACERT, config.sslcacert().get_value().c_str());
    }
    if (!config.sslclientcert().get_value().empty()) {
        handle_set_opt(h.get(), LRO_SSLCLIENTCERT, config.sslclientcert().get_value().c_str());
    }
    if (!config.sslclientkey().get_value().empty()) {
        handle_set_opt(h.get(), LRO_SSLCLIENTKEY, config.sslclientkey().get_value().c_str());
    }

    handle_set_opt(h.get(), LRO_PROGRESSCB, static_cast<LrProgressCb>(progress_cb));
    handle_set_opt(h.get(), LRO_PROGRESSDATA, callbacks.get());
    handle_set_opt(h.get(), LRO_FASTESTMIRRORCB, static_cast<LrFastestMirrorCb>(fastest_mirror_cb));
    handle_set_opt(h.get(), LRO_FASTESTMIRRORDATA, callbacks.get());

#ifdef LRO_SUPPORTS_CACHEDIR
    // If zchunk is enabled, set librepo cache dir
    if (config.get_main_config().zchunk().get_value()) {
        handle_set_opt(h.get(), LRO_CACHEDIR, config.basecachedir().get_value().c_str());
    }
#endif

    auto minrate = config.minrate().get_value();
    handle_set_opt(h.get(), LRO_LOWSPEEDLIMIT, static_cast<long>(minrate));

    auto maxspeed = config.throttle().get_value();
    if (maxspeed > 0 && maxspeed <= 1) {
        maxspeed *= static_cast<float>(config.bandwidth().get_value());
    }
    if (maxspeed != 0 && maxspeed < static_cast<float>(minrate)) {
        throw RuntimeError(
            _("Maximum download speed is lower than minimum. "
              "Please change configuration of minrate or throttle"));
    }
    handle_set_opt(h.get(), LRO_MAXSPEED, static_cast<int64_t>(maxspeed));

    long timeout = config.timeout().get_value();
    if (timeout > 0) {
        handle_set_opt(h.get(), LRO_CONNECTTIMEOUT, timeout);
        handle_set_opt(h.get(), LRO_LOWSPEEDTIME, timeout);
    } else {
        handle_set_opt(h.get(), LRO_CONNECTTIMEOUT, LRO_CONNECTTIMEOUT_DEFAULT);
        handle_set_opt(h.get(), LRO_LOWSPEEDTIME, LRO_LOWSPEEDTIME_DEFAULT);
    }

    if (!config.proxy().empty() && !config.proxy().get_value().empty()) {
        handle_set_opt(h.get(), LRO_PROXY, config.proxy().get_value().c_str());
    }

    // set proxy authorization methods
    auto proxy_auth_methods = string_to_proxy_auth_methods(config.proxy_auth_method().get_value());
    handle_set_opt(h.get(), LRO_PROXYAUTHMETHODS, static_cast<long>(proxy_auth_methods));

    if (!config.proxy_username().empty()) {
        userpwd = config.proxy_username().get_value();
        if (!userpwd.empty()) {
            userpwd = format_user_pass_string(userpwd, config.proxy_password().get_value(), true);
            handle_set_opt(h.get(), LRO_PROXYUSERPWD, userpwd.c_str());
        }
    }

    auto sslverify = config.sslverify().get_value() ? 1L : 0L;
    handle_set_opt(h.get(), LRO_SSLVERIFYHOST, sslverify);
    handle_set_opt(h.get(), LRO_SSLVERIFYPEER, sslverify);

    return h;
}

// TODO(jrohel): - replace gpgme by something else?
static void gpg_import_key(gpgme_ctx_t context, int key_fd, Logger & logger) {
    gpg_error_t gpg_err;
    gpgme_data_t key_data;

    gpg_err = gpgme_data_new_from_fd(&key_data, key_fd);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        auto msg = fmt::format(_("{}: gpgme_data_new_from_fd(): {}"), __func__, gpgme_strerror(gpg_err));
        logger.debug(msg);
        throw LrException(LRE_GPGERROR, msg);
    }

    gpg_err = gpgme_op_import(context, key_data);
    gpgme_data_release(key_data);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        auto msg = fmt::format(_("{}: gpgme_op_import(): {}"), __func__, gpgme_strerror(gpg_err));
        logger.debug(msg);
        throw LrException(LRE_GPGERROR, msg);
    }
}

static void gpg_import_key(gpgme_ctx_t context, std::vector<char> key, Logger & logger) {
    gpg_error_t gpg_err;
    gpgme_data_t key_data;

    gpg_err = gpgme_data_new_from_mem(&key_data, key.data(), key.size(), 0);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        auto msg = fmt::format(_("{}: gpgme_data_new_from_fd(): {}"), __func__, gpgme_strerror(gpg_err));
        logger.debug(msg);
        throw LrException(LRE_GPGERROR, msg);
    }

    gpg_err = gpgme_op_import(context, key_data);
    gpgme_data_release(key_data);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        auto msg = fmt::format(_("{}: gpgme_op_import(): {}"), __func__, gpgme_strerror(gpg_err));
        logger.debug(msg);
        throw LrException(LRE_GPGERROR, msg);
    }
}

static std::vector<Key> rawkey2infos(int fd, Logger & logger) {
    gpg_error_t gpg_err;

    std::vector<Key> key_infos;
    gpgme_ctx_t ctx;
    gpgme_new(&ctx);
    std::unique_ptr<std::remove_pointer<gpgme_ctx_t>::type> context(ctx);

    // set GPG home dir
    char tmpdir[] = "/tmp/tmpdir.XXXXXX";
    char * dir = mkdtemp(tmpdir);
    if (!dir) {
        throw std::runtime_error("mkdtemp failed");
    }

    std::unique_ptr<char, std::function<void(char *)>> tmp_dir_remover{
        tmpdir, [](char * tmpdir) { std::filesystem::remove_all(tmpdir); }};
    gpg_err = gpgme_ctx_set_engine_info(ctx, GPGME_PROTOCOL_OpenPGP, nullptr, tmpdir);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        auto msg = fmt::format(_("{}: gpgme_ctx_set_engine_info(): {}"), __func__, gpgme_strerror(gpg_err));
        logger.debug(msg);
        throw LrException(LRE_GPGERROR, msg);
    }

    gpg_import_key(ctx, fd, logger);

    gpgme_key_t key;
    gpg_err = gpgme_op_keylist_start(ctx, nullptr, 0);
    while (gpg_err == GPG_ERR_NO_ERROR) {
        gpg_err = gpgme_op_keylist_next(ctx, &key);
        if (gpg_err) {
            break;
        }

        // _extract_signing_subkey
        auto subkey = key->subkeys;
        while (subkey && !key->subkeys->can_sign) {
            subkey = subkey->next;
        }
        if (subkey)
            key_infos.emplace_back(key, subkey);
        gpgme_key_release(key);
    }
    if (gpg_err_code(gpg_err) != GPG_ERR_EOF) {
        gpgme_op_keylist_end(ctx);
        auto msg = fmt::format(_("can not list keys: {}"), gpgme_strerror(gpg_err));
        logger.debug(msg);
        throw LrException(LRE_GPGERROR, msg);
    }
    gpgme_set_armor(ctx, 1);
    for (auto & key_info : key_infos) {
        gpgme_data_t sink;
        gpgme_data_new(&sink);
        gpgme_op_export(ctx, key_info.get_id().c_str(), 0, sink);
        gpgme_data_rewind(sink);

        char buf[4096];
        ssize_t readed;
        do {
            readed = gpgme_data_read(sink, buf, sizeof(buf));
            if (readed > 0)
                key_info.raw_key.insert(key_info.raw_key.end(), buf, buf + sizeof(buf));
        } while (readed == sizeof(buf));
    }
    return key_infos;
}

static std::vector<std::string> keyids_from_pubring(const std::string & gpg_dir, Logger & logger) {
    gpg_error_t gpg_err;

    std::vector<std::string> keyids;

    struct stat sb;
    if (stat(gpg_dir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        gpgme_ctx_t ctx;
        gpgme_new(&ctx);
        std::unique_ptr<std::remove_pointer<gpgme_ctx_t>::type> context(ctx);

        // set GPG home dir
        gpg_err = gpgme_ctx_set_engine_info(ctx, GPGME_PROTOCOL_OpenPGP, nullptr, gpg_dir.c_str());
        if (gpg_err != GPG_ERR_NO_ERROR) {
            auto msg = fmt::format(_("{}: gpgme_ctx_set_engine_info(): {}"), __func__, gpgme_strerror(gpg_err));
            logger.debug(msg);
            throw LrException(LRE_GPGERROR, msg);
        }

        gpgme_key_t key;
        gpg_err = gpgme_op_keylist_start(ctx, nullptr, 0);
        while (gpg_err == GPG_ERR_NO_ERROR) {
            gpg_err = gpgme_op_keylist_next(ctx, &key);
            if (gpg_err) {
                break;
            }

            // _extract_signing_subkey
            auto subkey = key->subkeys;
            while (subkey && !key->subkeys->can_sign) {
                subkey = subkey->next;
            }
            if (subkey)
                keyids.push_back(subkey->keyid);
            gpgme_key_release(key);
        }
        if (gpg_err_code(gpg_err) != GPG_ERR_EOF) {
            gpgme_op_keylist_end(ctx);
            auto msg = fmt::format(_("can not list keys: {}"), gpgme_strerror(gpg_err));
            logger.debug(msg);
            throw LrException(LRE_GPGERROR, msg);
        }
    }
    return keyids;
}

// download key from URL
std::vector<Key> Repo::Impl::retrieve(const std::string & url) {
    auto & logger = base->get_logger();
    char tmp_key_file[] = "/tmp/repokey.XXXXXX";
    auto fd = mkstemp(tmp_key_file);
    if (fd == -1) {
        auto msg = fmt::format(
            "Error creating temporary file \"{}\": {}", tmp_key_file, std::system_category().message(errno));
        logger.debug(msg);
        throw LrException(LRE_GPGERROR, msg);
    }
    unlink(tmp_key_file);
    std::unique_ptr<int, std::function<void(int *)>> tmp_file_closer{&fd, [](int * fd) { close(*fd); }};

    try {
        download_url(url.c_str(), fd);
    } catch (const LrExceptionWithSourceUrl & e) {
        auto msg = fmt::format(_("Failed to retrieve GPG key for repo '{}': {}"), id, e.what());
        throw RuntimeError(msg);
    }
    lseek(fd, SEEK_SET, 0);
    auto key_infos = rawkey2infos(fd, logger);
    for (auto & key : key_infos)
        key.url = url;
    return key_infos;
}

/// Creates the '/run/user/$UID' directory if it doesn't exist. If this
/// directory exists, gpgagent will create its sockets under
/// '/run/user/$UID/gnupg'.
///
/// If this directory doesn't exist, gpgagent will create its sockets in gpg
/// home directory, which is under '/var/cache/yum/metadata/' and this was
/// causing trouble with container images, see [1].
///
/// Previous solution was to send the agent a "KILLAGENT" message, but that
/// would cause a race condition with calling gpgme_release(), see [2], [3],
/// [4].
///
/// Since the agent doesn't clean up its sockets properly, by creating this
/// directory we make sure they are in a place that is not causing trouble with
/// container images.
///
/// [1] https://bugzilla.redhat.com/show_bug.cgi?id=1650266
/// [2] https://bugzilla.redhat.com/show_bug.cgi?id=1769831
/// [3] https://github.com/rpm-software-management/microdnf/issues/50
/// [4] https://bugzilla.redhat.com/show_bug.cgi?id=1781601
static void ensure_socket_dir_exists(Logger & logger) {
    std::string dirname = "/run/user/" + std::to_string(getuid());
    int res = mkdir(dirname.c_str(), 0700);
    if (res != 0 && errno != EEXIST) {
        logger.debug(fmt::format("Failed to create directory \"{}\": {} - {}", dirname, errno, strerror(errno)));
    }
}

void Repo::Impl::import_repo_keys() {
    auto & logger = base->get_logger();

    auto gpg_dir = get_cachedir() + "/pubring";
    auto known_keys = keyids_from_pubring(gpg_dir, logger);
    ensure_socket_dir_exists(logger);
    for (const auto & gpgkeyUrl : config.gpgkey().get_value()) {
        auto key_infos = retrieve(gpgkeyUrl);
        for (auto & key_info : key_infos) {
            if (std::find(known_keys.begin(), known_keys.end(), key_info.get_id()) != known_keys.end()) {
                logger.debug(fmt::format(_("repo {}: 0x{} already imported"), id, key_info.get_id()));
                continue;
            }

            if (callbacks) {
                if (!callbacks->repokey_import(
                        key_info.get_id(),
                        key_info.get_user_id(),
                        key_info.get_fingerprint(),
                        key_info.url,
                        key_info.get_timestamp()))
                    continue;
            }

            struct stat sb;
            if (stat(gpg_dir.c_str(), &sb) != 0 || !S_ISDIR(sb.st_mode))
                mkdir(gpg_dir.c_str(), 0777);

            gpgme_ctx_t ctx;
            gpgme_new(&ctx);
            std::unique_ptr<std::remove_pointer<gpgme_ctx_t>::type> context(ctx);

            // set GPG home dir
            auto gpg_err = gpgme_ctx_set_engine_info(ctx, GPGME_PROTOCOL_OpenPGP, nullptr, gpg_dir.c_str());
            if (gpg_err != GPG_ERR_NO_ERROR) {
                auto msg = fmt::format(_("{}: gpgme_ctx_set_engine_info(): {}"), __func__, gpgme_strerror(gpg_err));
                logger.debug(msg);
                throw LrException(LRE_GPGERROR, msg);
            }

            gpg_import_key(ctx, key_info.raw_key, logger);

            logger.debug(fmt::format(_("repo {}: imported key 0x{}."), id, key_info.get_id()));
        }
    }
}

std::unique_ptr<LrResult> Repo::Impl::lr_handle_perform(
    LrHandle * handle, const std::string & dest_directory, bool set_gpg_home_dir) {
    if (set_gpg_home_dir) {
        auto pubringdir = get_cachedir() + "/pubring";
        handle_set_opt(handle, LRO_GNUPGHOMEDIR, pubringdir.c_str());
    }

    // Start and end is called only if progress callback is set in handle.
    LrProgressCb progressFunc;
    handle_get_info(handle, LRI_PROGRESSCB, &progressFunc);

    add_countme_flag(handle);

    std::unique_ptr<LrResult> result;
    bool ret;
    bool bad_gpg = false;
    do {
        if (callbacks && progressFunc)
            callbacks->start(
                !config.name().get_value().empty() ? config.name().get_value().c_str()
                                                   : (!id.empty() ? id.c_str() : "unknown"));

        GError * err_p{nullptr};
        result.reset(lr_result_init());
        ret = ::lr_handle_perform(handle, result.get(), &err_p);
        std::unique_ptr<GError> err(err_p);

        if (callbacks && progressFunc)
            callbacks->end();

        if (ret || bad_gpg || err_p->code != LRE_BADGPG) {
            if (!ret) {
                std::string source;
                if (config.metalink().empty() || (source = config.metalink().get_value()).empty()) {
                    if (config.mirrorlist().empty() || (source = config.mirrorlist().get_value()).empty()) {
                        bool first = true;
                        for (const auto & url : config.baseurl().get_value()) {
                            if (first)
                                first = false;
                            else
                                source += ", ";
                            source += url;
                        }
                    }
                }
                throw LrExceptionWithSourceUrl(err->code, err->message, source);
            }
            break;
        }
        bad_gpg = true;
        import_repo_keys();
        std::filesystem::remove_all(dest_directory + "/" + METADATA_RELATIVE_DIR);
    } while (true);

    return result;
}

void Repo::Impl::load_cache() {
    std::unique_ptr<LrHandle> h(lr_handle_init_local());
    std::unique_ptr<LrResult> r;

    // Fetch data
    r = lr_handle_perform(h.get(), get_cachedir(), config.repo_gpgcheck().get_value());

    char ** mirrors;
    LrYumRepo * yum_repo;
    LrYumRepoMd * yum_repomd;
    handle_get_info(h.get(), LRI_MIRRORS, &mirrors);
    result_get_info(r.get(), LRR_YUM_REPO, &yum_repo);
    result_get_info(r.get(), LRR_YUM_REPOMD, &yum_repomd);

    // Populate repo
    repomd_fn = yum_repo->repomd;
    metadata_paths.clear();
    for (auto * elem = yum_repo->paths; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto yumrepopath = static_cast<LrYumRepoPath *>(elem->data);
            metadata_paths.emplace(yumrepopath->type, yumrepopath->path);
        }
    }

    content_tags.clear();
    for (auto elem = yum_repomd->content_tags; elem; elem = g_slist_next(elem)) {
        if (elem->data)
            content_tags.emplace_back(static_cast<const char *>(elem->data));
    }

    distro_tags.clear();
    for (auto elem = yum_repomd->distro_tags; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto distro_tag = static_cast<LrYumDistroTag *>(elem->data);
            if (distro_tag->tag)
                distro_tags.emplace_back(distro_tag->cpeid, distro_tag->tag);
        }
    }

    metadata_locations.clear();
    for (auto elem = yum_repomd->records; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto rec = static_cast<LrYumRepoMdRecord *>(elem->data);
            metadata_locations.emplace_back(rec->type, rec->location_href);
        }
    }

    if (auto c_revision = yum_repomd->revision) {
        revision = c_revision;
    }
    max_timestamp = static_cast<int>(lr_yum_repomd_get_highest_timestamp(yum_repomd, nullptr));

    // Load timestamp unless explicitly expired
    if (timestamp != 0) {
        timestamp = mtime(get_metadata_path(MD_FILENAME_PRIMARY).c_str());
    }
    g_strfreev(this->mirrors);
    this->mirrors = mirrors;
}

bool Repo::Impl::try_load_cache() {
    try {
        load_cache();
    } catch (std::exception & ex) {
        return false;
    }
    return true;
}

/// The countme flag will be added once (and only once) in every position of
/// a sliding time window (COUNTME_WINDOW) that starts at COUNTME_OFFSET and
/// moves along the time axis, by one length at a time, in such a way that
/// the current point in time always stays within:
///
/// UNIX epoch                    now
/// |                             |
/// |---*-----|-----|-----|-----[-*---]---> time
///     |                       ~~~~~~~
///     COUNTME_OFFSET          COUNTME_WINDOW
///
/// This is to align the time window with an absolute point in time rather
/// than the last counting event (which could facilitate tracking across
/// multiple such events).
void Repo::Impl::add_countme_flag(LrHandle * handle) {
    auto & logger = base->get_logger();

    // Bail out if not counting or not running as root (since the persistdir is
    // only root-writable)
    if (!config.countme().get_value() || getuid() != 0)
        return;

    // Bail out if not a remote handle
    long local;
    handle_get_info(handle, LRI_LOCAL, &local);
    if (local)
        return;

    // Bail out if no metalink or mirrorlist is defined
    auto & metalink = config.metalink();
    auto & mirrorlist = config.mirrorlist();
    if ((metalink.empty() || metalink.get_value().empty()) && (mirrorlist.empty() || mirrorlist.get_value().empty()))
        return;

    // Load the cookie
    std::string fname = get_persistdir() + "/" + COUNTME_COOKIE;
    int ver = COUNTME_VERSION;    // file format version (for future use)
    time_t epoch = 0;             // position of first-ever counted window
    time_t win = COUNTME_OFFSET;  // position of last counted window
    int budget = -1;              // budget for this window (-1 = generate)
    std::ifstream(fname) >> ver >> epoch >> win >> budget;

    // Bail out if the window has not advanced since
    time_t now = time(nullptr);
    time_t delta = now - win;
    if (delta < COUNTME_WINDOW) {
        logger.debug(fmt::format("countme: no event for {}: window already counted", id));
        return;
    }

    // Evenly distribute the probability of the counting event over the first N
    // requests in this window (where N = COUNTME_BUDGET), by defining a random
    // "budget" of ordinary requests that we first have to spend.  This ensures
    // that no particular request is special and thus no privacy loss is
    // incurred by adding the flag within N requests.
    if (budget < 0) {
        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_int_distribution<int> dist(1, COUNTME_BUDGET);
        budget = dist(gen);
    }
    budget--;
    if (!budget) {
        // Budget exhausted, counting!

        // Compute the position of this window
        win = now - (delta % COUNTME_WINDOW);
        if (!epoch)
            epoch = win;
        // Window step (0 at epoch)
        int64_t step = (win - epoch) / COUNTME_WINDOW;

        // Compute the bucket we are in
        unsigned int i;
        for (i = 0; i < COUNTME_BUCKETS.size(); ++i)
            if (step < COUNTME_BUCKETS[i])
                break;
        int bucket = i + 1;  // Buckets are indexed from 1

        // Set the flag
        std::string flag = "countme=" + std::to_string(bucket);
        handle_set_opt(handle, LRO_ONETIMEFLAG, flag.c_str());
        logger.debug(fmt::format("countme: event triggered for {}: bucket {}", id, bucket));

        // Request a new budget
        budget = -1;
    } else {
        logger.debug(fmt::format("countme: no event for {}: budget to spend: {}", id, budget));
    }

    // Save the cookie
    std::ofstream(fname) << COUNTME_VERSION << " " << epoch << " " << win << " " << budget;
}

// Use metalink to check whether our metadata are still current.
bool Repo::Impl::is_metalink_in_sync() {
    auto & logger = base->get_logger();
    char tmpdir[] = "/tmp/tmpdir.XXXXXX";
    char * dir = mkdtemp(tmpdir);
    if (!dir) {
        throw std::runtime_error("mkdtemp failed");
    }

    std::unique_ptr<char, std::function<void(char *)>> tmp_dir_remover{
        tmpdir, [](char * tmpdir) { std::filesystem::remove_all(tmpdir); }};

    std::unique_ptr<LrHandle> h(lr_handle_init_remote(tmpdir));

    handle_set_opt(h.get(), LRO_FETCHMIRRORS, 1L);
    auto r = lr_handle_perform(h.get(), tmpdir, false);
    LrMetalink * metalink;
    handle_get_info(h.get(), LRI_METALINK, &metalink);
    if (!metalink) {
        logger.debug(fmt::format(_("reviving: repo '{}' skipped, no metalink."), id));
        return false;
    }

    // check all recognized hashes
    auto chksum_free = [](Chksum * ptr) { solv_chksum_free(ptr, nullptr); };
    struct hashInfo {
        const LrMetalinkHash * lr_metalink_hash;
        std::unique_ptr<Chksum, decltype(chksum_free)> chksum;
    };
    std::vector<hashInfo> hashes;
    for (auto hash = metalink->hashes; hash; hash = hash->next) {
        auto lr_metalink_hash = static_cast<const LrMetalinkHash *>(hash->data);
        for (auto algorithm : RECOGNIZED_CHKSUMS) {
            if (strcmp(lr_metalink_hash->type, algorithm) == 0)
                hashes.push_back({lr_metalink_hash, {nullptr, chksum_free}});
        }
    }
    if (hashes.empty()) {
        logger.debug(fmt::format(_("reviving: repo '{}' skipped, no usable hash."), id));
        return false;
    }

    for (auto & hash : hashes) {
        auto chk_type = solv_chksum_str2type(hash.lr_metalink_hash->type);
        hash.chksum.reset(solv_chksum_create(chk_type));
    }

    std::ifstream repomd(repomd_fn, std::ifstream::binary);
    char buf[4096];
    int readed;
    while ((readed = static_cast<int>(repomd.readsome(buf, sizeof(buf)))) > 0) {
        for (auto & hash : hashes)
            solv_chksum_add(hash.chksum.get(), buf, readed);
    }

    for (auto & hash : hashes) {
        int chksumLen;
        auto chksum = solv_chksum_get(hash.chksum.get(), &chksumLen);
        char chksumHex[chksumLen * 2 + 1];
        solv_bin2hex(chksum, chksumLen, chksumHex);
        if (strcmp(chksumHex, hash.lr_metalink_hash->value) != 0) {
            logger.debug(
                fmt::format(_("reviving: failed for '{}', mismatched {} sum."), id, hash.lr_metalink_hash->type));
            return false;
        }
    }

    logger.debug(fmt::format(_("reviving: '{}' can be revived - metalink checksums match."), id));
    return true;
}

// Use repomd to check whether our metadata are still current.
bool Repo::Impl::is_repomd_in_sync() {
    auto & logger = base->get_logger();
    LrYumRepo * yum_repo;
    char tmpdir[] = "/tmp/tmpdir.XXXXXX";
    char * dir = mkdtemp(tmpdir);
    if (!dir) {
        throw std::runtime_error("mkdtemp failed");
    }

    std::unique_ptr<char, std::function<void(char *)>> tmp_dir_remover{
        tmpdir, [](char * tmpdir) { std::filesystem::remove_all(tmpdir); }};

    const char * dlist[] = LR_YUM_REPOMDONLY;

    std::unique_ptr<LrHandle> h(lr_handle_init_remote(tmpdir));

    handle_set_opt(h.get(), LRO_YUMDLIST, dlist);
    auto r = lr_handle_perform(h.get(), tmpdir, config.repo_gpgcheck().get_value());
    result_get_info(r.get(), LRR_YUM_REPO, &yum_repo);

    auto same = have_files_same_content_noexcept(repomd_fn.c_str(), yum_repo->repomd);
    if (same)
        logger.debug(fmt::format(_("reviving: '{}' can be revived - repomd matches."), id));
    else
        logger.debug(fmt::format(_("reviving: failed for '{}', mismatched repomd."), id));
    return same;
}

bool Repo::Impl::is_in_sync() {
    if (!config.metalink().empty() && !config.metalink().get_value().empty())
        return is_metalink_in_sync();
    return is_repomd_in_sync();
}


void Repo::Impl::fetch(const std::string & destdir, std::unique_ptr<LrHandle> && h) {
    auto repodir = destdir + "/" + METADATA_RELATIVE_DIR;
    if (g_mkdir_with_parents(destdir.c_str(), 0755) == -1) {
        const char * err_txt = strerror(errno);
        throw RuntimeError(fmt::format(_("Cannot create repo destination directory \"{}\": {}"), destdir, err_txt));
    }
    auto tmpdir = destdir + "/tmpdir.XXXXXX";
    if (!mkdtemp(&tmpdir.front())) {
        const char * err_txt = strerror(errno);
        throw RuntimeError(
            fmt::format(_("Cannot create repo temporary directory \"{}\": {}"), tmpdir.c_str(), err_txt));
    }
    std::unique_ptr<char, std::function<void(char *)>> tmp_dir_remover{
        &tmpdir.front(), [](char * tmpdir) { std::filesystem::remove_all(tmpdir); }};
    auto tmprepodir = tmpdir + "/" + METADATA_RELATIVE_DIR;

    handle_set_opt(h.get(), LRO_DESTDIR, tmpdir.c_str());
    auto r = lr_handle_perform(h.get(), tmpdir, config.repo_gpgcheck().get_value());

    std::filesystem::remove_all(repodir);
    if (g_mkdir_with_parents(repodir.c_str(), 0755) == -1) {
        const char * err_txt = strerror(errno);
        throw RuntimeError(fmt::format(_("Cannot create directory \"{}\": {}"), repodir, err_txt));
    }
    // move all downloaded object from tmpdir to destdir
    if (auto * dir = opendir(tmpdir.c_str())) {
        std::unique_ptr<DIR, std::function<void(DIR *)>> tmp_dir_remover{dir, [](DIR * dir) { closedir(dir); }};

        while (auto ent = readdir(dir)) {
            auto el_name = ent->d_name;
            if (el_name[0] == '.' && (el_name[1] == '\0' || (el_name[1] == '.' && el_name[2] == '\0'))) {
                continue;
            }
            auto target_element = destdir + "/" + el_name;
            std::filesystem::remove_all(target_element);
            auto tempElement = tmpdir + "/" + el_name;
            try {
                move_recursive(tempElement.c_str(), target_element.c_str());
            } catch (const std::filesystem::filesystem_error & ex) {
                std::string err_txt = fmt::format(
                    _("Cannot rename directory \"{}\" to \"{}\": {}"), tempElement, target_element, ex.what());
                throw RuntimeError(err_txt);
            }
        }
    }
}

void Repo::Impl::download_metadata(const std::string & destdir) {
    std::unique_ptr<LrHandle> h(lr_handle_init_remote(nullptr));
    handle_set_opt(h.get(), LRO_YUMDLIST, LR_RPMMD_FULL);
    fetch(destdir, std::move(h));
}

bool Repo::Impl::load() {
    auto & logger = base->get_logger();
    try {
        if (!get_metadata_path(MD_FILENAME_PRIMARY).empty() || try_load_cache()) {
            reset_metadata_expired();
            if (!expired || sync_strategy == SyncStrategy::ONLY_CACHE || sync_strategy == SyncStrategy::LAZY) {
                logger.debug(fmt::format(_("repo: using cache for: {}"), id));
                return false;
            }

            if (is_in_sync()) {
                // the expired metadata still reflect the origin:
                utimes(get_metadata_path(MD_FILENAME_PRIMARY).c_str(), nullptr);
                expired = false;
                return true;
            }
        }
        if (sync_strategy == SyncStrategy::ONLY_CACHE) {
            auto msg = fmt::format(_("Cache-only enabled but no cache for '{}'"), id);
            throw RuntimeError(msg);
        }

        logger.debug(fmt::format(_("repo: downloading from remote: {}"), id));
        const auto cache_dir = get_cachedir();
        fetch(cache_dir, lr_handle_init_remote(nullptr));
        timestamp = -1;
        load_cache();
    } catch (const LrExceptionWithSourceUrl & e) {
        auto msg = fmt::format(_("Failed to download metadata for repo '{}': {}"), id, e.what());
        throw RuntimeError(msg);
    }
    expired = false;
    return true;
}

std::string Repo::Impl::get_hash() const {
    std::string tmp;
    if (config.metalink().empty() || (tmp = config.metalink().get_value()).empty()) {
        if (config.mirrorlist().empty() || (tmp = config.mirrorlist().get_value()).empty()) {
            if (!config.baseurl().get_value().empty())
                tmp = config.baseurl().get_value()[0];
            if (tmp.empty())
                tmp = id;
        }
    }

    auto chksum_obj = solv_chksum_create(REPOKEY_TYPE_SHA256);
    solv_chksum_add(chksum_obj, tmp.c_str(), static_cast<int>(tmp.length()));
    int chksum_len;
    auto chksum = solv_chksum_get(chksum_obj, &chksum_len);
    static constexpr int USE_CHECKSUM_BYTES = 8;
    if (chksum_len < USE_CHECKSUM_BYTES) {
        solv_chksum_free(chksum_obj, nullptr);
        throw RuntimeError(_("getCachedir(): Computation of SHA256 failed"));
    }
    char chksum_cstr[USE_CHECKSUM_BYTES * 2 + 1];
    solv_bin2hex(chksum, USE_CHECKSUM_BYTES, chksum_cstr);
    solv_chksum_free(chksum_obj, nullptr);

    return id + "-" + chksum_cstr;
}

std::string Repo::Impl::get_cachedir() const {
    auto repodir(config.basecachedir().get_value());
    if (repodir.back() != '/')
        repodir.push_back('/');
    return repodir + get_hash();
}

std::string Repo::Impl::get_persistdir() const {
    auto persdir(config.get_main_config().persistdir().get_value());
    if (persdir.back() != '/')
        persdir.push_back('/');
    std::string result = persdir + "repos/" + get_hash();
    if (g_mkdir_with_parents(result.c_str(), 0755) == -1) {
        const char * err_txt = strerror(errno);
        throw RuntimeError(fmt::format(_("Cannot create persistdir \"{}\": {}"), result, err_txt));
    }
    return result;
}

int64_t Repo::Impl::get_age() const {
    return time(nullptr) - mtime(get_metadata_path(MD_FILENAME_PRIMARY).c_str());
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

void Repo::Impl::download_url(const char * url, int fd) {
    if (callbacks)
        callbacks->start(
            !config.name().get_value().empty() ? config.name().get_value().c_str()
                                               : (!id.empty() ? id.c_str() : "unknown"));

    GError * err_p{nullptr};
    lr_download_url(get_cached_handle(), url, fd, &err_p);
    std::unique_ptr<GError> err(err_p);

    if (callbacks)
        callbacks->end();

    if (err)
        throw LrExceptionWithSourceUrl(err->code, err->message, url);
}

void Repo::Impl::set_http_headers(const char * headers[]) {
    if (!headers) {
        http_headers.reset();
        return;
    }
    size_t headers_count = 0;
    while (headers[headers_count])
        ++headers_count;
    http_headers.reset(new char * [headers_count + 1] {});
    for (size_t i = 0; i < headers_count; ++i) {
        http_headers[i] = new char[strlen(headers[i]) + 1];
        strcpy(http_headers[i], headers[i]);
    }
}

const char * const * Repo::Impl::get_http_headers() const {
    return http_headers.get();
}

bool Repo::fresh() {
    return p_impl->timestamp >= 0;
}

void Repo::Impl::reset_metadata_expired() {
    if (expired || config.metadata_expire().get_value() == -1)
        return;
    if (config.get_main_config().check_config_file_age().get_value() && !repo_file_path.empty() &&
        mtime(repo_file_path.c_str()) > mtime(get_metadata_path(MD_FILENAME_PRIMARY).c_str()))
        expired = true;
    else
        expired = get_age() > config.metadata_expire().get_value();
}


/// Returns a librepo handle, set as per the repo options.
/// Note that destdir is None, and the handle is cached.
LrHandle * Repo::Impl::get_cached_handle() {
    if (!handle)
        handle = lr_handle_init_remote(nullptr);
    handle_set_opt(handle.get(), LRO_HTTPHEADER, http_headers.get());
    return handle.get();
}

void Repo::Impl::attach_libsolv_repo(LibsolvRepo * libsolv_repo) {
    if (this->libsolv_repo_ext.repo) {
        throw LogicError("libdnf::rpm::Repo: Some libsolv repository is already attached.");
    }

    libsolv_repo->appdata = owner;  // The libsolvRepo references back to us.
    libsolv_repo->subpriority = -owner->get_cost();
    libsolv_repo->priority = -owner->get_priority();
    this->libsolv_repo_ext.repo = libsolv_repo;
}

void Repo::Impl::detach_libsolv_repo() {
    if (!libsolv_repo_ext.repo) {
        // Nothing to do, libsolvRepo is not attached.
        return;
    }

    libsolv_repo_ext.repo->appdata = nullptr;  // Removes reference to this object from libsolvRepo.
    this->libsolv_repo_ext.repo = nullptr;
}

void Repo::set_max_mirror_tries(int max_mirror_tries) {
    p_impl->max_mirror_tries = max_mirror_tries;
}

int64_t Repo::get_timestamp() const {
    return p_impl->timestamp;
}

int Repo::get_max_timestamp() {
    return p_impl->max_timestamp;
}

void Repo::set_preserve_remote_time(bool preserve_remote_time) {
    p_impl->preserve_remote_time = preserve_remote_time;
}

bool Repo::get_preserve_remote_time() const {
    return p_impl->preserve_remote_time;
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

const std::string & Repo::get_revision() const {
    return p_impl->revision;
}

std::string Repo::get_cachedir() const {
    return p_impl->get_cachedir();
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

void Repo::download_url(const char * url, int fd) {
    p_impl->download_url(url, fd);
}

void Repo::set_http_headers(const char * headers[]) {
    p_impl->set_http_headers(headers);
}

const char * const * Repo::get_http_headers() const {
    return p_impl->get_http_headers();
}

std::vector<std::string> Repo::get_mirrors() const {
    std::vector<std::string> mirrors;
    if (p_impl->mirrors) {
        for (auto mirror = p_impl->mirrors; *mirror; ++mirror)
            mirrors.emplace_back(*mirror);
    }
    return mirrors;
}

Id Repo::Impl::add_rpm_package(const std::string & fn, bool add_with_hdrid) {
    auto c_fn = fn.c_str();
    is_readable_rpm(c_fn);

    int flags = REPO_REUSE_REPODATA | REPO_NO_INTERNALIZE;
    if (add_with_hdrid) {
        flags |= RPM_ADD_WITH_HDRID | RPM_ADD_WITH_SHA256SUM;
    }

    Id new_id = repo_add_rpm(libsolv_repo_ext.repo, c_fn, flags);
    if (new_id == 0) {
        throw RuntimeError(_("Failed to read RPM: ") + fn);
    }
    libsolv_repo_ext.set_needs_internalizing();
    return new_id;
}

bool LibsolvRepoExt::is_one_piece() const {
    for (auto i = repo->start; i < repo->end; ++i)
        if (repo->pool->solvables[i].repo != repo)
            return false;
    return true;
}

void LibsolvRepoExt::internalize() {
    if (!needs_internalizing) {
        return;
    }
    repo_internalize(repo);
    needs_internalizing = false;
}

int PackageTargetCB::end([[maybe_unused]] TransferStatus status, [[maybe_unused]] const char * msg) {
    return 0;
}
int PackageTargetCB::progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) {
    return 0;
}
int PackageTargetCB::mirror_failure([[maybe_unused]] const char * msg, [[maybe_unused]] const char * url) {
    return 0;
}

class PackageTarget::Impl {
public:
    Impl(
        Repo * repo,
        const char * relative_url,
        const char * dest,
        int chks_type,
        const char * chksum,
        int64_t expected_size,
        const char * base_url,
        bool resume,
        int64_t byte_range_start,
        int64_t byte_range_end,
        PackageTargetCB * callbacks);

    Impl(
        ConfigMain * cfg,
        const char * relative_url,
        const char * dest,
        int chks_type,
        const char * chksum,
        int64_t expected_size,
        const char * base_url,
        bool resume,
        int64_t byte_range_start,
        int64_t byte_range_end,
        PackageTargetCB * callbacks,
        const char * http_headers[]);

    void download();

    ~Impl();

    PackageTargetCB * callbacks;

    std::unique_ptr<LrPackageTarget> lr_pkg_target;

private:
    void init(
        LrHandle * handle,
        const char * relative_url,
        const char * dest,
        int chks_type,
        const char * chksum,
        int64_t expected_size,
        const char * base_url,
        bool resume,
        int64_t byte_range_start,
        int64_t byte_range_end);

    static int end_cb(void * data, LrTransferStatus status, const char * msg);
    static int progress_cb(void * data, double total_to_download, double downloaded);
    static int mirror_failure_cb(void * data, const char * msg, const char * url);

    std::unique_ptr<LrHandle> lrHandle;
};


int PackageTarget::Impl::end_cb(void * data, LrTransferStatus status, const char * msg) {
    if (!data)
        return 0;
    auto cb_object = static_cast<PackageTargetCB *>(data);
    return cb_object->end(static_cast<PackageTargetCB::TransferStatus>(status), msg);
}

int PackageTarget::Impl::progress_cb(void * data, double total_to_download, double downloaded) {
    if (!data)
        return 0;
    auto cb_object = static_cast<PackageTargetCB *>(data);
    return cb_object->progress(total_to_download, downloaded);
}

int PackageTarget::Impl::mirror_failure_cb(void * data, const char * msg, const char * url) {
    if (!data)
        return 0;
    auto cb_object = static_cast<PackageTargetCB *>(data);
    return cb_object->mirror_failure(msg, url);
}


static LrHandle * new_handle(ConfigMain * config) {
    LrHandle * h = ::lr_handle_init();
    const char * user_agent = USER_AGENT;
    // see dnf.repo.Repo._handle_new_remote() how to pass
    if (config) {
        user_agent = config->user_agent().get_value().c_str();
        auto minrate = config->minrate().get_value();
        handle_set_opt(h, LRO_LOWSPEEDLIMIT, static_cast<long>(minrate));

        auto maxspeed = config->throttle().get_value();
        if (maxspeed > 0 && maxspeed <= 1) {
            maxspeed *= static_cast<float>(config->bandwidth().get_value());
        }
        if (maxspeed != 0 && maxspeed < static_cast<float>(minrate))
            throw RuntimeError(
                _("Maximum download speed is lower than minimum. "
                  "Please change configuration of minrate or throttle"));
        handle_set_opt(h, LRO_MAXSPEED, static_cast<int64_t>(maxspeed));

        if (!config->proxy().empty() && !config->proxy().get_value().empty())
            handle_set_opt(h, LRO_PROXY, config->proxy().get_value().c_str());

        // set proxy authorization methods
        auto proxy_auth_methods = string_to_proxy_auth_methods(config->proxy_auth_method().get_value());
        handle_set_opt(h, LRO_PROXYAUTHMETHODS, static_cast<long>(proxy_auth_methods));

        if (!config->proxy_username().empty()) {
            auto userpwd = config->proxy_username().get_value();
            if (!userpwd.empty()) {
                userpwd = format_user_pass_string(userpwd, config->proxy_password().get_value(), true);
                handle_set_opt(h, LRO_PROXYUSERPWD, userpwd.c_str());
            }
        }

        // setup ssl stuff
        if (!config->sslcacert().get_value().empty()) {
            handle_set_opt(h, LRO_SSLCACERT, config->sslcacert().get_value().c_str());
        }
        if (!config->sslclientcert().get_value().empty()) {
            handle_set_opt(h, LRO_SSLCLIENTCERT, config->sslclientcert().get_value().c_str());
        }
        if (!config->sslclientkey().get_value().empty()) {
            handle_set_opt(h, LRO_SSLCLIENTKEY, config->sslclientkey().get_value().c_str());
        }

        auto sslverify = config->sslverify().get_value() ? 1L : 0L;
        handle_set_opt(h, LRO_SSLVERIFYHOST, sslverify);
        handle_set_opt(h, LRO_SSLVERIFYPEER, sslverify);
    }
    handle_set_opt(h, LRO_USERAGENT, user_agent);
    return h;
}

PackageTarget::ChecksumType PackageTarget::checksum_type(const std::string & name) {
    return static_cast<ChecksumType>(lr_checksum_type(name.c_str()));
}

void PackageTarget::download_packages(std::vector<PackageTarget *> & targets, bool fail_fast) {
    // Convert vector to GSList
    GSList * list{nullptr};
    for (auto it = targets.rbegin(); it != targets.rend(); ++it)
        list = g_slist_prepend(list, (*it)->p_impl->lr_pkg_target.get());
    std::unique_ptr<GSList, decltype(&g_slist_free)> list_guard(list, &g_slist_free);

    LrPackageDownloadFlag flags = static_cast<LrPackageDownloadFlag>(0);
    if (fail_fast)
        flags = static_cast<LrPackageDownloadFlag>(flags | LR_PACKAGEDOWNLOAD_FAILFAST);

    GError * err_p{nullptr};
    lr_download_packages(list, flags, &err_p);
    std::unique_ptr<GError> err(err_p);

    if (err)
        throw_exception(std::move(err));
}


PackageTarget::Impl::~Impl() = default;

PackageTarget::Impl::Impl(
    Repo * repo,
    const char * relative_url,
    const char * dest,
    int chks_type,
    const char * chksum,
    int64_t expected_size,
    const char * base_url,
    bool resume,
    int64_t byte_range_start,
    int64_t byte_range_end,
    PackageTargetCB * callbacks)
    : callbacks(callbacks) {
    init(
        repo->p_impl->get_cached_handle(),
        relative_url,
        dest,
        chks_type,
        chksum,
        expected_size,
        base_url,
        resume,
        byte_range_start,
        byte_range_end);
}

PackageTarget::Impl::Impl(
    ConfigMain * cfg,
    const char * relative_url,
    const char * dest,
    int chks_type,
    const char * chksum,
    int64_t expected_size,
    const char * base_url,
    bool resume,
    int64_t byte_range_start,
    int64_t byte_range_end,
    PackageTargetCB * callbacks,
    const char * http_headers[])
    : callbacks(callbacks) {
    lrHandle.reset(new_handle(cfg));
    handle_set_opt(lrHandle.get(), LRO_HTTPHEADER, http_headers);
    handle_set_opt(lrHandle.get(), LRO_REPOTYPE, LR_YUMREPO);
    init(
        lrHandle.get(),
        relative_url,
        dest,
        chks_type,
        chksum,
        expected_size,
        base_url,
        resume,
        byte_range_start,
        byte_range_end);
}

void PackageTarget::Impl::init(
    LrHandle * handle,
    const char * relative_url,
    const char * dest,
    int chks_type,
    const char * chksum,
    int64_t expected_size,
    const char * base_url,
    bool resume,
    int64_t byte_range_start,
    int64_t byte_range_end) {
    auto lr_chks_type = static_cast<LrChecksumType>(chks_type);

    if (resume && byte_range_start) {
        auto msg = _("resume cannot be used simultaneously with the byterangestart param");
        throw RuntimeError(msg);
    }

    GError * err_p{nullptr};
    lr_pkg_target.reset(lr_packagetarget_new_v3(
        handle,
        relative_url,
        dest,
        lr_chks_type,
        chksum,
        expected_size,
        base_url,
        resume,
        progress_cb,
        callbacks,
        end_cb,
        mirror_failure_cb,
        byte_range_start,
        byte_range_end,
        &err_p));
    std::unique_ptr<GError> err(err_p);

    if (!lr_pkg_target) {
        auto msg = fmt::format(_("PackageTarget initialization failed: {}"), err->message);
        throw RuntimeError(msg);
    }
}

PackageTarget::PackageTarget(
    Repo * repo,
    const char * relative_url,
    const char * dest,
    int chks_type,
    const char * chksum,
    int64_t expected_size,
    const char * base_url,
    bool resume,
    int64_t byte_range_start,
    int64_t byte_range_end,
    PackageTargetCB * callbacks)
    : p_impl(new Impl(
          repo,
          relative_url,
          dest,
          chks_type,
          chksum,
          expected_size,
          base_url,
          resume,
          byte_range_start,
          byte_range_end,
          callbacks)) {}

PackageTarget::PackageTarget(
    ConfigMain * cfg,
    const char * relative_url,
    const char * dest,
    int chks_type,
    const char * chksum,
    int64_t expected_size,
    const char * base_url,
    bool resume,
    int64_t byte_range_start,
    int64_t byte_range_end,
    PackageTargetCB * callbacks,
    const char * http_headers[])
    : p_impl(new Impl(
          cfg,
          relative_url,
          dest,
          chks_type,
          chksum,
          expected_size,
          base_url,
          resume,
          byte_range_start,
          byte_range_end,
          callbacks,
          http_headers)) {}


PackageTarget::~PackageTarget() {}

PackageTargetCB * PackageTarget::get_callbacks() {
    return p_impl->callbacks;
}

const char * PackageTarget::get_err() {
    return p_impl->lr_pkg_target->err;
}

void Downloader::download_url(ConfigMain * cfg, const char * url, int fd) {
    std::unique_ptr<LrHandle> lr_handle(new_handle(cfg));
    GError * err_p{nullptr};
    lr_download_url(lr_handle.get(), url, fd, &err_p);
    std::unique_ptr<GError> err(err_p);

    if (err)
        throw_exception(std::move(err));
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
        throw RuntimeError(fmt::format(_("Cannot open {}: {}"), file_path, g_strerror(errno)));
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
        throw RuntimeError(fmt::format(_("Log handler with id {} doesn't exist"), uid));
    }

    // Remove the handler and free the data
    lr_log_datas.erase(it);
}

void LibrepoLog::remove_all_handlers() {
    std::lock_guard<std::mutex> guard(lr_log_datas_mutex);
    lr_log_datas.clear();
}

}  // namespace libdnf::rpm
