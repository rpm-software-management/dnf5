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

#include "repo_downloader.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/fs/temp.hpp"
#include "utils/fs/utils.hpp"
#include "utils/string.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/conf/const.hpp"
#include "libdnf/repo/repo_errors.hpp"

#include <librepo/librepo.h>
#include <solv/chksum.h>
#include <solv/util.h>

#include <filesystem>
#include <fstream>
#include <random>


#define METADATA_RELATIVE_DIR "repodata"
#define PACKAGES_RELATIVE_DIR "packages"
#define METALINK_FILENAME     "metalink.xml"
#define MIRRORLIST_FILENAME   "mirrorlist"
#define RECOGNIZED_CHKSUMS \
    { "sha512", "sha256" }


namespace libdnf::repo {

static void str_vector_to_char_array(const std::vector<std::string> & vec, const char * arr[]) {
    for (size_t i = 0; i < vec.size(); ++i) {
        arr[i] = vec[i].c_str();
    }
    arr[vec.size()] = nullptr;
}

static LrYumRepo * get_yum_repo(LibrepoResult & result) {
    LrYumRepo * yum_repo;
    result.get_info(LRR_YUM_REPO, &yum_repo);
    return yum_repo;
}

static LrYumRepoMd * get_yum_repomd(LibrepoResult & result) {
    LrYumRepoMd * yum_repomd;
    result.get_info(LRR_YUM_REPOMD, &yum_repomd);
    return yum_repomd;
}


static int progress_cb(void * data, double total_to_download, double downloaded) {
    if (!data) {
        return 0;
    }
    auto cb_object = static_cast<libdnf::repo::RepoCallbacks *>(data);
    return cb_object->progress(total_to_download, downloaded);
}

static void fastest_mirror_cb(void * data, LrFastestMirrorStages stage, void * ptr) {
    if (!data) {
        return;
    }
    auto cb_object = static_cast<libdnf::repo::RepoCallbacks *>(data);
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
    cb_object->fastest_mirror(static_cast<libdnf::repo::RepoCallbacks::FastestMirrorStage>(stage), msg);
}

static int mirror_failure_cb(void * data, const char * msg, const char * url, const char * metadata) {
    if (!data) {
        return 0;
    }
    auto cb_object = static_cast<libdnf::repo::RepoCallbacks *>(data);
    return cb_object->handle_mirror_failure(msg, url, metadata);
};


/// Recursive renames/moves file/directory.
/// Implements copy and remove fallback.
// TODO(lukash) move to utils/fs
static void move_recursive(const std::filesystem::path & src, const std::filesystem::path & dest) {
    try {
        std::filesystem::rename(src, dest);
    } catch (const std::filesystem::filesystem_error & ex) {
        std::filesystem::copy(src, dest, std::filesystem::copy_options::recursive);
        std::filesystem::remove_all(src);
    }
}


LibrepoError::LibrepoError(std::unique_ptr<GError> && lr_error)
    : Error(M_("Librepo error: {}"), std::string(lr_error->message)),
      code(lr_error->code) {}


RepoDownloader::RepoDownloader(const libdnf::BaseWeakPtr & base, const ConfigRepo & config, Repo::Type repo_type)
    : base(base),
      config(config),
      repo_type(repo_type),
      pgp(base, config) {}

RepoDownloader::~RepoDownloader() = default;


void RepoDownloader::download_metadata(const std::string & destdir) try {
    std::filesystem::create_directories(destdir);
    libdnf::utils::fs::TempDir tmpdir(destdir, "tmpdir");

    LibrepoHandle h(init_remote_handle(tmpdir.get_path().c_str()));
    perform(h, tmpdir.get_path(), config.repo_gpgcheck().get_value());

    // move all downloaded object from tmpdir to destdir
    for (auto & dir : std::filesystem::directory_iterator(tmpdir.get_path())) {
        auto tmp_item = dir.path();

        auto target_item = destdir / tmp_item.filename();
        std::filesystem::remove_all(target_item);

        move_recursive(tmp_item, target_item);
    }
} catch (const std::runtime_error & e) {
    auto src = get_source_info();
    throw_with_nested(RepoDownloadError(
        M_("Failed to download metadata ({}: \"{}\") for repository \"{}\""), src.first, src.second, config.get_id()));
}


// Use metalink to check whether our metadata are still current.
bool RepoDownloader::is_metalink_in_sync() try {
    auto & logger = *base->get_logger();

    libdnf::utils::fs::TempDir tmpdir("tmpdir");

    LibrepoHandle h(init_remote_handle(tmpdir.get_path().c_str()));

    h.set_opt(LRO_FETCHMIRRORS, 1L);
    perform(h, tmpdir.get_path().c_str(), false);
    LrMetalink * metalink;
    h.get_info(LRI_METALINK, &metalink);
    if (!metalink) {
        logger.trace("Sync check: repo \"{}\" skipped, no metalink", config.get_id());
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
        logger.trace("Sync check: repo \"{}\" skipped, no usable hash", config.get_id());
        return false;
    }

    for (auto & hash : hashes) {
        auto chk_type = solv_chksum_str2type(hash.lr_metalink_hash->type);
        hash.chksum.reset(solv_chksum_create(chk_type));
    }

    std::ifstream repomd(repomd_filename, std::ifstream::binary);
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
            logger.trace(
                "Sync check: failed for repo \"{}\", {} checksum mismatch",
                config.get_id(),
                hash.lr_metalink_hash->type);
            return false;
        }
    }

    logger.debug("Sync check: repo \"{}\" in sync, metalink checksums match", config.get_id());
    return true;
} catch (const std::runtime_error & e) {
    throw_with_nested(RepoDownloadError(
        M_("Error checking if metalink \"{}\" is in sync for repository \"{}\""),
        get_source_info().second,
        config.get_id()));
}


// Use repomd to check whether our metadata are still current.
bool RepoDownloader::is_repomd_in_sync() try {
    auto & logger = *base->get_logger();
    LrYumRepo * yum_repo;

    libdnf::utils::fs::TempDir tmpdir("tmpdir");

    const char * dlist[] = LR_YUM_REPOMDONLY;

    LibrepoHandle h(init_remote_handle(tmpdir.get_path().c_str()));

    h.set_opt(LRO_YUMDLIST, dlist);
    auto result = perform(h, tmpdir.get_path().c_str(), config.repo_gpgcheck().get_value());
    result.get_info(LRR_YUM_REPO, &yum_repo);

    auto same = utils::fs::have_files_same_content_noexcept(repomd_filename.c_str(), yum_repo->repomd);
    if (same)
        logger.debug("Sync check: repo \"{}\" in sync, repomd matches", config.get_id());
    else
        logger.trace("Sync check: failed for repo \"{}\", repomd mismatch", config.get_id());
    return same;
} catch (const std::runtime_error & e) {
    auto src = get_source_info();
    throw_with_nested(RepoDownloadError(
        M_("Error checking if repomd ({}: \"{}\") is in sync for repository \"{}\""),
        src.first,
        src.second,
        config.get_id()));
}


void RepoDownloader::load_local() try {
    LibrepoHandle h(init_local_handle());

    auto result = perform(h, config.get_cachedir(), config.repo_gpgcheck().get_value());

    repomd_filename = libdnf::utils::string::c_to_str(get_yum_repo(result)->repomd);

    // copy the mirrors out of the handle (handle_get_info() allocates new
    // space and passes ownership, so we copy twice in this case, as we want to
    // store a vector of strings)
    char ** lr_mirrors;
    h.get_info(LRI_MIRRORS, &lr_mirrors);
    if (lr_mirrors) {
        for (auto mirror = lr_mirrors; *mirror; ++mirror) {
            mirrors.emplace_back(*mirror);
        }
    }
    g_strfreev(lr_mirrors);

    revision = libdnf::utils::string::c_to_str(get_yum_repomd(result)->revision);

    GError * err_p{nullptr};
    // TODO(lukash) return time_t instead of converting to signed int
    max_timestamp = static_cast<int>(lr_yum_repomd_get_highest_timestamp(get_yum_repomd(result), &err_p));
    if (err_p != nullptr) {
        throw libdnf::repo::LibrepoError(std::unique_ptr<GError>(err_p));
    }

    for (auto elem = get_yum_repomd(result)->content_tags; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            content_tags.emplace_back(static_cast<const char *>(elem->data));
        }
    }

    for (auto elem = get_yum_repomd(result)->distro_tags; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto distro_tag = static_cast<LrYumDistroTag *>(elem->data);
            if (distro_tag->tag) {
                distro_tags.emplace_back(distro_tag->cpeid, distro_tag->tag);
            }
        }
    }

    for (auto elem = get_yum_repomd(result)->records; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto rec = static_cast<LrYumRepoMdRecord *>(elem->data);
            metadata_locations.emplace_back(rec->type, rec->location_href);
        }
    }

    for (auto * elem = get_yum_repo(result)->paths; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto yumrepopath = static_cast<LrYumRepoPath *>(elem->data);
            metadata_paths[yumrepopath->type] = yumrepopath->path;
        }
    }
} catch (const std::runtime_error & e) {
    throw_with_nested(RepoDownloadError(M_("Error loading local metadata for repository \"{}\""), config.get_id()));
}


/// Returns a librepo handle, set as per the repo options.
/// Note that destdir is None, and the handle is cached.
// TODO(jrohel) The librepo handle callbacks are not set. If librepo itself downloads an extra file
//              (eg metalink) we won't know about it.
LibrepoHandle & RepoDownloader::get_cached_handle() {
    if (!handle) {
        handle = init_remote_handle(nullptr, true, false);
    }
    apply_http_headers(*handle);
    return *handle;
}


void RepoDownloader::set_callbacks(std::unique_ptr<libdnf::repo::RepoCallbacks> && cbs) noexcept {
    callbacks = std::move(cbs);
    pgp.set_callbacks(callbacks.get());
}


const std::string & RepoDownloader::get_metadata_path(const std::string & metadata_type) const {
    auto it = metadata_paths.end();

    if (config.get_main_config().zchunk().get_value() && !utils::string::ends_with(metadata_type, "_zck")) {
        it = metadata_paths.find(metadata_type + "_zck");
    }

    if (it == metadata_paths.end()) {
        it = metadata_paths.find(metadata_type);
    }

    static const std::string empty;
    return it != metadata_paths.end() ? it->second : empty;
}


LibrepoHandle RepoDownloader::init_local_handle() {
    LibrepoHandle h;

    common_handle_setup(h);

    std::string cachedir = config.get_cachedir();
    h.set_opt(LRO_DESTDIR, cachedir.c_str());
    const char * urls[] = {cachedir.c_str(), nullptr};
    h.set_opt(LRO_URLS, urls);
    h.set_opt(LRO_LOCAL, 1L);

    return h;
}

LibrepoHandle RepoDownloader::init_remote_handle(const char * destdir, bool mirror_setup, bool set_callbacks) {
    LibrepoHandle h;
    h.init_remote(config);

    common_handle_setup(h);

    apply_http_headers(h);

    h.set_opt(LRO_DESTDIR, destdir);

    enum class Source { NONE, METALINK, MIRRORLIST } source{Source::NONE};
    std::string tmp;
    if (!config.metalink().empty() && !(tmp = config.metalink().get_value()).empty()) {
        source = Source::METALINK;
    } else if (!config.mirrorlist().empty() && !(tmp = config.mirrorlist().get_value()).empty()) {
        source = Source::MIRRORLIST;
    }
    if (source != Source::NONE) {
        if (mirror_setup) {
            if (source == Source::METALINK) {
                h.set_opt(LRO_METALINKURL, tmp.c_str());
            } else {
                h.set_opt(LRO_MIRRORLISTURL, tmp.c_str());
                // YUM-DNF compatibility hack. YUM guessed by content of keyword "metalink" if
                // mirrorlist is really mirrorlist or metalink)
                if (tmp.find("metalink") != tmp.npos)
                    h.set_opt(LRO_METALINKURL, tmp.c_str());
            }

            h.set_opt(LRO_FASTESTMIRROR, config.fastestmirror().get_value() ? 1L : 0L);

            auto fastest_mirror_cache_dir = config.basecachedir().get_value();
            if (fastest_mirror_cache_dir.back() != '/') {
                fastest_mirror_cache_dir.push_back('/');
            }
            fastest_mirror_cache_dir += "fastestmirror.cache";
            h.set_opt(LRO_FASTESTMIRRORCACHE, fastest_mirror_cache_dir.c_str());
        } else {
            // use already resolved mirror list
            const char * c_mirrors[mirrors.size() + 1];
            str_vector_to_char_array(mirrors, c_mirrors);
            h.set_opt(LRO_URLS, c_mirrors);
        }
    } else if (!config.baseurl().get_value().empty()) {
        const char * urls[config.baseurl().get_value().size() + 1];
        str_vector_to_char_array(config.baseurl().get_value(), urls);
        h.set_opt(LRO_URLS, urls);
    } else {
        throw RepoDownloadError(
            M_("No valid source (baseurl, mirrorlist or metalink) found for repository \"{}\""), config.get_id());
    }

    if (set_callbacks) {
        h.set_opt(LRO_PROGRESSCB, static_cast<LrProgressCb>(progress_cb));
        h.set_opt(LRO_PROGRESSDATA, callbacks.get());
        h.set_opt(LRO_FASTESTMIRRORCB, static_cast<LrFastestMirrorCb>(fastest_mirror_cb));
        h.set_opt(LRO_FASTESTMIRRORDATA, callbacks.get());
        h.set_opt(LRO_HMFCB, static_cast<LrHandleMirrorFailureCb>(mirror_failure_cb));
    }

    return h;
}

void RepoDownloader::common_handle_setup(LibrepoHandle & h) {
    std::vector<const char *> dlist;

    auto optional_metadata = get_optional_metadata();

    dlist.push_back(MD_FILENAME_PRIMARY);
#ifdef MODULEMD
    dlist.push_back(MD_FILENAME_MODULES);
#endif
    if (optional_metadata.extract(libdnf::METADATA_TYPE_FILELISTS)) {
        dlist.push_back(MD_FILENAME_FILELISTS);
    }
    if (optional_metadata.extract(libdnf::METADATA_TYPE_OTHER)) {
        dlist.push_back(MD_FILENAME_OTHER);
    }
    if (optional_metadata.extract(libdnf::METADATA_TYPE_PRESTO)) {
        dlist.push_back(MD_FILENAME_PRESTODELTA);
    }
    if (optional_metadata.extract(libdnf::METADATA_TYPE_COMPS)) {
        dlist.push_back(MD_FILENAME_GROUP_GZ);
    }
    if (optional_metadata.extract(libdnf::METADATA_TYPE_UPDATEINFO)) {
        dlist.push_back(MD_FILENAME_UPDATEINFO);
    }

    // download the rest metadata added by 3rd parties
    for (auto & item : optional_metadata) {
        dlist.push_back(item.c_str());
    }
    dlist.push_back(nullptr);
    h.set_opt(LRO_PRESERVETIME, static_cast<long>(preserve_remote_time));
    h.set_opt(LRO_REPOTYPE, LR_YUMREPO);
    h.set_opt(LRO_YUMDLIST, dlist.data());
    h.set_opt(LRO_INTERRUPTIBLE, 1L);
    h.set_opt(LRO_GPGCHECK, config.repo_gpgcheck().get_value());
    h.set_opt(LRO_MAXMIRRORTRIES, static_cast<long>(max_mirror_tries));
    h.set_opt(LRO_MAXPARALLELDOWNLOADS, config.max_parallel_downloads().get_value());

    LrUrlVars * repomd_substs = nullptr;
    repomd_substs = lr_urlvars_set(repomd_substs, MD_FILENAME_GROUP_GZ, MD_FILENAME_GROUP);
    h.set_opt(LRO_YUMSLIST, repomd_substs);

    LrUrlVars * substs = nullptr;
    for (const auto & item : substitutions) {
        substs = lr_urlvars_set(substs, item.first.c_str(), item.second.c_str());
    }
    h.set_opt(LRO_VARSUB, substs);

#ifdef LRO_SUPPORTS_CACHEDIR
    // If zchunk is enabled, set librepo cache dir
    if (config.get_main_config().zchunk().get_value()) {
        h.set_opt(LRO_CACHEDIR, config.basecachedir().get_value().c_str());
    }
#endif
}

void RepoDownloader::apply_http_headers(LibrepoHandle & handle) {
    if (http_headers.empty()) {
        handle.set_opt(LRO_HTTPHEADER, nullptr);
        return;
    }

    std::unique_ptr<char * [], std::function<void(char **)>> lr_headers {
        nullptr, [](char ** ptr) {
            for (auto item = ptr; *item != nullptr; ++item) {
                delete[] * item;
            }
            delete[] ptr;
        }
    };

    lr_headers.reset(new char * [http_headers.size() + 1] {});

    for (size_t i = 0; i < http_headers.size(); ++i) {
        const auto & header = http_headers[i];
        lr_headers[i] = new char[header.size() + 1];
        strcpy(lr_headers[i], header.c_str());
    }

    handle.set_opt(LRO_HTTPHEADER, lr_headers.get());
}


LibrepoResult RepoDownloader::perform(
    LibrepoHandle & handle, const std::string & dest_directory, bool set_gpg_home_dir) {
    if (set_gpg_home_dir) {
        auto pubringdir = pgp.get_keyring_dir();
        handle.set_opt(LRO_GNUPGHOMEDIR, pubringdir.c_str());
    }

    // Start and end is called only if progress callback is set in handle.
    LrProgressCb progress_func;
    handle.get_info(LRI_PROGRESSCB, &progress_func);

    add_countme_flag(handle);

    bool bad_gpg = false;
    do {
        if (callbacks && progress_func) {
            callbacks->start(
                !config.name().get_value().empty() ? config.name().get_value().c_str()
                                                   : (!config.get_id().empty() ? config.get_id().c_str() : "unknown"));
        }

        try {
            auto result = handle.perform();
            // finished successfully
            if (callbacks && progress_func) {
                callbacks->end(nullptr);
            }
            return result;
        } catch (const LibrepoError & ex) {
            if (bad_gpg || ex.get_code() != LRE_BADGPG) {
                if (callbacks && progress_func) {
                    callbacks->end(ex.what());
                }
                throw;
            }
        }

        // TODO(lukash) we probably shouldn't call end() in this case
        if (callbacks && progress_func) {
            callbacks->end(nullptr);
        }

        bad_gpg = true;
        // TODO(lukash) this calls callbacks->{start,end}() for the second time, doesn't seem right
        import_repo_keys();
        std::filesystem::remove_all(dest_directory + "/" + METADATA_RELATIVE_DIR);
    } while (true);
}


void RepoDownloader::download_url(const char * url, int fd) {
    if (callbacks) {
        callbacks->start(
            !config.name().get_value().empty() ? config.name().get_value().c_str()
                                               : (!config.get_id().empty() ? config.get_id().c_str() : "unknown"));
    }

    GError * err_p{nullptr};
    bool res = lr_download_url(get_cached_handle().get(), url, fd, &err_p);

    if (res) {
        if (callbacks) {
            callbacks->end(nullptr);
        }
    } else {
        std::unique_ptr<GError> err(err_p);

        if (callbacks) {
            callbacks->end(err->message);
        }

        // TODO(lukash) does the error from librepo contain the URL or do we need to add it here somehow?
        throw LibrepoError(std::move(err));
    }
}


std::pair<std::string, std::string> RepoDownloader::get_source_info() const {
    if (!config.metalink().empty() && !config.metalink().get_value().empty()) {
        return {"metalink", config.metalink().get_value()};
    } else if (!config.mirrorlist().empty() && !config.mirrorlist().get_value().empty()) {
        return {"mirrorlist", config.mirrorlist().get_value()};
    } else {
        return {"baseurl", libdnf::utils::string::join(config.baseurl().get_value(), ", ")};
    }
}


void RepoDownloader::import_repo_keys() {
    for (const auto & gpgkey_url : config.gpgkey().get_value()) {
        auto tmp_file = libdnf::utils::fs::TempFile("repokey");

        download_url(gpgkey_url.c_str(), tmp_file.get_fd());

        lseek(tmp_file.get_fd(), SEEK_SET, 0);
        pgp.import_key(tmp_file.get_fd(), gpgkey_url);
    }
}


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
void RepoDownloader::add_countme_flag(LibrepoHandle & handle) {
    auto & logger = *base->get_logger();

    // Bail out if not counting or not running as root (since the persistdir is
    // only root-writable)
    if (!config.countme().get_value() || getuid() != 0)
        return;

    // Bail out if not a remote handle
    long local;
    handle.get_info(LRI_LOCAL, &local);
    if (local)
        return;

    // Bail out if no metalink or mirrorlist is defined
    auto & metalink = config.metalink();
    auto & mirrorlist = config.mirrorlist();
    if ((metalink.empty() || metalink.get_value().empty()) && (mirrorlist.empty() || mirrorlist.get_value().empty()))
        return;

    if (!std::filesystem::is_directory(config.get_persistdir())) {
        std::filesystem::create_directories(config.get_persistdir());
    }

    // Load the cookie
    std::filesystem::path file_path(config.get_persistdir());
    file_path /= COUNTME_COOKIE;

    int ver = COUNTME_VERSION;    // file format version (for future use)
    time_t epoch = 0;             // position of first-ever counted window
    time_t win = COUNTME_OFFSET;  // position of last counted window
    int budget = -1;              // budget for this window (-1 = generate)
    // TODO(lukash) ideally replace with utils::fs::File (via adding scanf() support?),
    // once we are able to test this (using CI stack tests)
    std::ifstream(file_path) >> ver >> epoch >> win >> budget;

    // Bail out if the window has not advanced since
    time_t now = time(nullptr);
    time_t delta = now - win;
    if (delta < COUNTME_WINDOW) {
        logger.trace("countme: no event for repo \"{}\": window already counted", config.get_id());
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
        uint32_t i;
        for (i = 0; i < COUNTME_BUCKETS.size(); ++i)
            if (step < COUNTME_BUCKETS[i])
                break;
        uint32_t bucket = i + 1;  // Buckets are indexed from 1

        // Set the flag
        std::string flag = "countme=" + std::to_string(bucket);
        handle.set_opt(LRO_ONETIMEFLAG, flag.c_str());
        logger.trace("countme: event triggered for repo \"{}\": bucket {}", config.get_id(), bucket);

        // Request a new budget
        budget = -1;
    } else {
        logger.trace("countme: no event for repo \"{}\": budget to spend: {}", config.get_id(), budget);
    }

    // Save the cookie
    utils::fs::File(file_path, "w").write(fmt::format("{} {} {} {}", COUNTME_VERSION, epoch, win, budget));
}


// TODO(jkolarik): currently all metadata are loaded for system repo, maybe we want it configurable?
std::unordered_set<std::string> RepoDownloader::get_optional_metadata() const {
    if (repo_type == Repo::Type::SYSTEM) {
        return libdnf::OPTIONAL_METADATA_TYPES;
    } else {
        return config.get_main_config().optional_metadata_types().get_value();
    }
}


//void Downloader::download_url(ConfigMain * cfg, const char * url, int fd) {
//    std::unique_ptr<LrHandle> lr_handle(new_remote_handle(*cfg));
//    GError * err_p{nullptr};
//    lr_download_url(lr_handle.get(), url, fd, &err_p);
//    std::unique_ptr<GError> err(err_p);
//
//    if (err)
//        throw LibrepoError(std::move(err));
//}

}  //namespace libdnf::repo
