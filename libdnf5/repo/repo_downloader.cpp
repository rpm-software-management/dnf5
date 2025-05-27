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

#include "utils/fs/utils.hpp"
#include "utils/string.hpp"

#include "libdnf5/conf/const.hpp"
#include "libdnf5/repo/repo.hpp"
#include "libdnf5/repo/repo_errors.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <fmt/format.h>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <librepo/librepo.h>
#include <solv/chksum.h>
#include <solv/util.h>
#include <sys/stat.h>

#include <filesystem>
#include <fstream>
#include <random>

#define RECOGNIZED_CHKSUMS {"sha512", "sha256"}

namespace std {

template <>
struct default_delete<LrMetadataTarget> {
    void operator()(LrMetadataTarget * ptr) noexcept { lr_metadatatarget_free(ptr); }
};

}  // namespace std

namespace libdnf5::repo {

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

int RepoDownloader::end_cb(void * data, LrTransferStatus status, const char * msg) {
    libdnf_assert(data != nullptr, "data in callback must be set");

    auto download_callback_data = static_cast<CallbackData *>(data);
    auto cb_status = static_cast<DownloadCallbacks::TransferStatus>(status);
    if (cb_status == DownloadCallbacks::TransferStatus::SUCCESSFUL && download_callback_data) {
        // move all downloaded object from tmpdir to destdir
        for (auto & dir :
             std::filesystem::directory_iterator(download_callback_data->temp_download_target->get_path())) {
            auto tmp_item = dir.path();

            auto target_item = download_callback_data->destination / tmp_item.filename();
            std::filesystem::remove_all(target_item);

            utils::fs::move_recursive(tmp_item, target_item);
        }

        if (download_callback_data->load_repo) {
            download_callback_data->load_repo(download_callback_data->repo.get(), false);
        }
    }
    if (auto * download_callbacks = download_callback_data->repo->get_base()->get_download_callbacks()) {
        return download_callbacks->end(download_callback_data->user_cb_data, cb_status, msg);
    }
    return 0;
}

int RepoDownloader::progress_cb(void * data, double total_to_download, double downloaded) {
    if (!data) {
        return 0;
    }
    auto download_callback_data = static_cast<CallbackData *>(data);
    if (auto * download_callbacks = download_callback_data->repo->get_base()->get_download_callbacks()) {
        // "total_to_download" and "downloaded" from librepo are related to the currently downloaded file.
        // We add the size of previously downloaded files.
        // ignore zero progress events at the beginning of the download, so we don't start with 100% progress
        if (total_to_download != 0 || download_callback_data->prev_total_to_download != 0) {
            if (total_to_download != download_callback_data->prev_total_to_download) {
                download_callback_data->prev_total_to_download = total_to_download;
                download_callback_data->sum_prev_downloaded += download_callback_data->prev_downloaded;
            }
            download_callback_data->prev_downloaded = downloaded;
        }
        total_to_download += download_callback_data->sum_prev_downloaded;
        downloaded += download_callback_data->sum_prev_downloaded;

        return download_callbacks->progress(download_callback_data->user_cb_data, total_to_download, downloaded);
    }
    return 0;
}

void RepoDownloader::fastest_mirror_cb(void * data, LrFastestMirrorStages stage, void * ptr) {
    if (!data) {
        return;
    }
    auto download_callback_data = static_cast<CallbackData *>(data);
    if (auto * download_callbacks = download_callback_data->repo->get_base()->get_download_callbacks()) {
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
        download_callbacks->fastest_mirror(
            download_callback_data->user_cb_data, static_cast<DownloadCallbacks::FastestMirrorStage>(stage), msg);
    }
}

int RepoDownloader::mirror_failure_cb(void * data, const char * msg, const char * url) {
    if (!data) {
        return 0;
    }
    auto download_callback_data = static_cast<CallbackData *>(data);
    if (auto * download_callbacks = download_callback_data->repo->get_base()->get_download_callbacks()) {
        // In this callback type we don't have the `metadata` type, pass NULL instead. The dnf5 callback accounts for this.
        return download_callbacks->mirror_failure(download_callback_data->user_cb_data, msg, url, NULL);
    }
    return 0;
};

int RepoDownloader::mirror_failure_cb(
    void * data, const char * msg, const char * url, [[maybe_unused]] const char * metadata) {
    if (!data) {
        return 0;
    }

    auto download_callback_data = static_cast<CallbackData *>(data);
    if (auto * download_callbacks = download_callback_data->repo->get_base()->get_download_callbacks()) {
        return download_callbacks->mirror_failure(download_callback_data->user_cb_data, msg, url, metadata);
    }
    return 0;
};


LibrepoError::LibrepoError(std::unique_ptr<GError> && lr_error)
    : Error(M_("Librepo error: {}"), std::string(lr_error->message)),
      code(lr_error->code) {}


void RepoDownloader::add(
    libdnf5::repo::Repo & repo, const std::string & destdir, std::function<repo_loading_func> load_repo) try {
    auto & cbd = callback_data.emplace_back();
    cbd.repo = repo.get_weak_ptr();
    cbd.destination = destdir;
    cbd.load_repo = load_repo;
}

std::unordered_map<Repo *, std::vector<std::string>> RepoDownloader::download() try {
    if (callback_data.empty()) {
        return {};
    }

    //TODO(amatej): If we make this API public should we take cacheonly option into account?
    //              PackageDownloader respects it.
    //              For repos it is currently handled in update_and_load_repos so we don't
    //              even attempt to download remote repos with cacheonly enabled.

    GError * err{nullptr};
    std::vector<std::tuple<RepoWeakPtr, std::unique_ptr<LrMetadataTarget>>> lr_targets;
    for (auto & cbd : callback_data) {
        std::filesystem::create_directories(cbd.destination);
        auto & download_data = cbd.repo->get_download_data();
        cbd.temp_download_target = libdnf5::utils::fs::TempDir{cbd.destination, "tmpdir"};

        auto * download_callbacks = cbd.repo->get_base()->get_download_callbacks();
        auto & config = cbd.repo->get_config();
        if (download_callbacks) {
            cbd.user_cb_data = download_callbacks->add_new_download(
                download_data.user_data,
                !config.get_name_option().get_value().empty()
                    ? config.get_name_option().get_value().c_str()
                    : (!config.get_id().empty() ? config.get_id().c_str() : "unknown"),
                -1);
            cbd.prev_total_to_download = 0;
            cbd.prev_downloaded = 0;
            cbd.sum_prev_downloaded = 0;
        }
        download_data.handle = init_remote_handle(*cbd.repo, cbd.temp_download_target->get_path().c_str(), true);
        download_data.handle->set_opt(LRO_FASTESTMIRRORCB, static_cast<LrFastestMirrorCb>(fastest_mirror_cb));
        download_data.handle->set_opt(LRO_FASTESTMIRRORDATA, &cbd);
        add_countme_flag(download_data, *download_data.handle);
        auto t = lr_metadatatarget_new2(
            download_data.handle->get(),
            &cbd,
            static_cast<LrProgressCb>(progress_cb),
            static_cast<LrMirrorFailureCb>(mirror_failure_cb),
            static_cast<LrEndCb>(end_cb),
            config.get_repo_gpgcheck_option().get_value() ? download_data.pgp.get_keyring_dir().c_str() : NULL,
            0);
        lr_targets.emplace_back(cbd.repo, t);
    }

    // Adding items to the end of GSList is slow. We go from the back and add items to the beginning.
    GSList * list{nullptr};
    for (auto it = lr_targets.rbegin(); it != lr_targets.rend(); ++it) {
        list = g_slist_prepend(list, std::get<1>(*it).get());
    }
    std::unique_ptr<GSList, decltype(&g_slist_free)> list_holder(list, &g_slist_free);


    if (!lr_download_metadata(list, &err)) {
        throw LibrepoError(std::unique_ptr<GError>(err));
    }

    std::unordered_map<Repo *, std::vector<std::string>> per_repo_errors;
    for (auto & [repo, target] : lr_targets) {
        std::vector<std::string> errors;
        for (GList * err = target->err; err; err = g_list_next(err)) {
            gchar * err_msg = (gchar *)err->data;
            errors.emplace_back(err_msg);
        }

        per_repo_errors[repo.get()] = errors;
    }

    return per_repo_errors;
} catch (const std::runtime_error & e) {
    libdnf5::throw_with_nested(RepoDownloadError(M_("Failed to download metadata")));
}

// Use metalink to check whether our metadata are still current.
bool RepoDownloader::is_metalink_in_sync(Repo & repo) try {
    auto & download_data = repo.get_download_data();
    auto & logger = *(download_data.base)->get_logger();
    auto & config = download_data.config;

    libdnf5::utils::fs::TempDir tmpdir("tmpdir");

    CallbackData cbd;
    cbd.repo = repo.get_weak_ptr();

    LibrepoHandle h(RepoDownloader::init_remote_handle(repo, tmpdir.get_path().c_str()));
    h.set_opt(LRO_FETCHMIRRORS, 1L);

    perform(download_data, h, false, &cbd);
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

    std::ifstream repomd(download_data.repomd_filename, std::ifstream::binary);
    char buf[4096];
    int readed;
    while ((readed = static_cast<int>(repomd.readsome(buf, sizeof(buf)))) > 0) {
        for (auto & hash : hashes)
            solv_chksum_add(hash.chksum.get(), buf, readed);
    }

    for (auto & hash : hashes) {
        int chksumLen;
        auto chksum = solv_chksum_get(hash.chksum.get(), &chksumLen);
        std::vector<char> chksumHex(static_cast<std::size_t>(chksumLen * 2 + 1), '\0');
        solv_bin2hex(chksum, chksumLen, chksumHex.data());
        if (strcmp(chksumHex.data(), hash.lr_metalink_hash->value) != 0) {
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
    libdnf5::throw_with_nested(RepoDownloadError(
        M_("Error checking if metalink \"{}\" is in sync for repository \"{}\""),
        repo.get_download_data().get_source_info().second,
        repo.get_download_data().config.get_id()));
}

// Use repomd to check whether our metadata are still current.
bool RepoDownloader::is_repomd_in_sync(Repo & repo) try {
    auto & download_data = repo.get_download_data();
    auto & logger = *(download_data.base)->get_logger();
    auto & config = download_data.config;
    LrYumRepo * yum_repo;

    libdnf5::utils::fs::TempDir tmpdir("tmpdir");

    const char * dlist[] = LR_YUM_REPOMDONLY;

    LibrepoHandle h(RepoDownloader::init_remote_handle(repo, tmpdir.get_path().c_str()));

    h.set_opt(LRO_YUMDLIST, dlist);
    CallbackData cbd;
    cbd.repo = repo.get_weak_ptr();
    auto result = perform(download_data, h, config.get_repo_gpgcheck_option().get_value(), &cbd);
    result.get_info(LRR_YUM_REPO, &yum_repo);

    auto same = utils::fs::have_files_same_content_noexcept(download_data.repomd_filename.c_str(), yum_repo->repomd);
    if (same)
        logger.debug("Sync check: repo \"{}\" in sync, repomd matches", config.get_id());
    else
        logger.trace("Sync check: failed for repo \"{}\", repomd mismatch", config.get_id());
    return same;
} catch (const std::runtime_error & e) {
    auto src = repo.get_download_data().get_source_info();
    libdnf5::throw_with_nested(RepoDownloadError(
        M_("Error checking if repomd ({}: \"{}\") is in sync for repository \"{}\""),
        src.first,
        src.second,
        repo.get_config().get_id()));
}

void RepoDownloader::load_local(DownloadData & download_data) try {
    auto & config = download_data.config;
    LibrepoHandle h(RepoDownloader::init_local_handle(download_data));

    auto result = perform(download_data, h, config.get_repo_gpgcheck_option().get_value(), NULL);

    download_data.repomd_filename = libdnf5::utils::string::c_to_str(get_yum_repo(result)->repomd);

    // copy the mirrors out of the handle (handle_get_info() allocates new
    // space and passes ownership, so we copy twice in this case, as we want to
    // store a vector of strings)
    char ** lr_mirrors;
    h.get_info(LRI_MIRRORS, &lr_mirrors);
    if (lr_mirrors) {
        for (auto mirror = lr_mirrors; *mirror; ++mirror) {
            download_data.mirrors.emplace_back(*mirror);
        }
    }
    g_strfreev(lr_mirrors);

    download_data.revision = libdnf5::utils::string::c_to_str(get_yum_repomd(result)->revision);

    GError * err_p{nullptr};
    // TODO(lukash) return time_t instead of converting to signed int
    download_data.max_timestamp = static_cast<int>(lr_yum_repomd_get_highest_timestamp(get_yum_repomd(result), &err_p));
    if (err_p != nullptr) {
        throw libdnf5::repo::LibrepoError(std::unique_ptr<GError>(err_p));
    }

    for (auto elem = get_yum_repomd(result)->content_tags; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            download_data.content_tags.emplace_back(static_cast<const char *>(elem->data));
        }
    }

    for (auto elem = get_yum_repomd(result)->distro_tags; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto distro_tag = static_cast<LrYumDistroTag *>(elem->data);
            if (distro_tag->tag) {
                download_data.distro_tags.emplace_back(distro_tag->cpeid, distro_tag->tag);
            }
        }
    }

    for (auto elem = get_yum_repomd(result)->records; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto rec = static_cast<LrYumRepoMdRecord *>(elem->data);
            download_data.metadata_locations.emplace_back(rec->type, rec->location_href);
        }
    }

    for (auto * elem = get_yum_repo(result)->paths; elem; elem = g_slist_next(elem)) {
        if (elem->data) {
            auto yumrepopath = static_cast<LrYumRepoPath *>(elem->data);
            download_data.metadata_paths[yumrepopath->type] = yumrepopath->path;
        }
    }
} catch (const std::runtime_error & e) {
    libdnf5::throw_with_nested(
        RepoDownloadError(M_("Error loading local metadata for repository \"{}\""), download_data.config.get_id()));
}

/// Returns a librepo handle, set as per the repo options.
/// Note that destdir is None, and the handle is cached.
// TODO(jrohel) The librepo handle callbacks are not set. If librepo itself downloads an extra file
//              (eg metalink) we won't know about it.
LibrepoHandle & RepoDownloader::get_cached_handle(Repo & repo) {
    auto & donwload_data = repo.get_download_data();
    if (!donwload_data.handle) {
        donwload_data.handle = RepoDownloader::init_remote_handle(repo, nullptr, true);
    }
    apply_http_headers(donwload_data, *donwload_data.handle);
    return *donwload_data.handle;
}

LibrepoHandle RepoDownloader::init_local_handle(const DownloadData & download_data) {
    LibrepoHandle h;

    common_handle_setup(h, download_data);

    std::string cachedir = download_data.config.get_cachedir();
    h.set_opt(LRO_DESTDIR, cachedir.c_str());
    const char * urls[] = {cachedir.c_str(), nullptr};
    h.set_opt(LRO_URLS, urls);
    h.set_opt(LRO_LOCAL, 1L);

    return h;
}

LibrepoHandle RepoDownloader::init_remote_handle(Repo & repo, const char * destdir, bool mirror_setup) {
    auto & download_data = repo.get_download_data();
    LibrepoHandle h;
    h.init_remote(download_data.config);

    RepoDownloader::common_handle_setup(h, download_data);

    apply_http_headers(download_data, h);

    h.set_opt(LRO_DESTDIR, destdir);

    auto & config = download_data.config;

    enum class Source { NONE, METALINK, MIRRORLIST } source{Source::NONE};
    std::string tmp;
    if (!config.get_metalink_option().empty() && !(tmp = config.get_metalink_option().get_value()).empty()) {
        source = Source::METALINK;
    } else if (!config.get_mirrorlist_option().empty() && !(tmp = config.get_mirrorlist_option().get_value()).empty()) {
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

            h.set_opt(LRO_FASTESTMIRROR, config.get_fastestmirror_option().get_value() ? 1L : 0L);

            auto fastest_mirror_cache_dir = config.get_basecachedir_option().get_value();
            if (fastest_mirror_cache_dir.back() != '/') {
                fastest_mirror_cache_dir.push_back('/');
            }
            fastest_mirror_cache_dir += "fastestmirror.cache";
            h.set_opt(LRO_FASTESTMIRRORCACHE, fastest_mirror_cache_dir.c_str());
        } else {
            // use already resolved mirror list
            std::vector<const char *> c_mirrors(download_data.mirrors.size() + 1, nullptr);
            str_vector_to_char_array(download_data.mirrors, c_mirrors.data());
            h.set_opt(LRO_URLS, c_mirrors.data());
        }
    } else if (!config.get_baseurl_option().get_value().empty()) {
        std::vector<const char *> urls(config.get_baseurl_option().get_value().size() + 1, nullptr);
        str_vector_to_char_array(config.get_baseurl_option().get_value(), urls.data());
        h.set_opt(LRO_URLS, urls.data());
    } else {
        throw RepoDownloadError(
            M_("No valid source (baseurl, mirrorlist or metalink) found for repository \"{}\""), config.get_id());
    }

    return h;
}

void RepoDownloader::configure_handle_dlist(LibrepoHandle & h, std::set<std::string> && optional_metadata) {
    if (optional_metadata.extract(libdnf5::METADATA_TYPE_ALL)) {
        h.set_opt(LRO_YUMDLIST, LR_RPMMD_FULL);
    } else {
        std::vector<const char *> dlist;
        dlist.push_back(MD_FILENAME_PRIMARY);
#ifdef WITH_MODULEMD
        dlist.push_back(MD_FILENAME_MODULES);
#endif
        if (optional_metadata.extract(libdnf5::METADATA_TYPE_FILELISTS)) {
            dlist.push_back(MD_FILENAME_FILELISTS);
        }
        if (optional_metadata.extract(libdnf5::METADATA_TYPE_OTHER)) {
            dlist.push_back(MD_FILENAME_OTHER);
        }
        if (optional_metadata.extract(libdnf5::METADATA_TYPE_PRESTO)) {
            dlist.push_back(MD_FILENAME_PRESTODELTA);
        }
        if (optional_metadata.extract(libdnf5::METADATA_TYPE_COMPS)) {
            dlist.push_back(MD_FILENAME_GROUP_GZ);
        }
        if (optional_metadata.extract(libdnf5::METADATA_TYPE_UPDATEINFO)) {
            dlist.push_back(MD_FILENAME_UPDATEINFO);
        }

        // download the rest metadata added by 3rd parties
        for (auto & item : optional_metadata) {
            // the appstream metadata is a "virtual" type, the list
            // of the types is read from the repomd file
            if (item.compare(libdnf5::METADATA_TYPE_APPSTREAM) != 0)
                dlist.push_back(item.c_str());
        }
        if (optional_metadata.extract(libdnf5::METADATA_TYPE_APPSTREAM)) {
            // ideally, the repomd.xml file should be read and every type matching is_appstream_metadata_type()
            // would be added from it, but the content is not known at this point and when it is known, then
            // it's too late, thus declare some "expected" types to be downloaded here
            dlist.push_back("appstream");
            dlist.push_back("appstream-icons");
            dlist.push_back("appstream-icons-48x48");
            dlist.push_back("appstream-icons-48x48@2");
            dlist.push_back("appstream-icons-64x64");
            dlist.push_back("appstream-icons-64x64@2");
            dlist.push_back("appstream-icons-128x128");
            dlist.push_back("appstream-icons-128x128@2");
            // consult the prefixes with the is_appstream_metadata_type()
            dlist.push_back("appdata");
            dlist.push_back("appdata-icons");
            dlist.push_back("appdata-icons-48x48");
            dlist.push_back("appdata-icons-48x48@2");
            dlist.push_back("appdata-icons-64x64");
            dlist.push_back("appdata-icons-64x64@2");
            dlist.push_back("appdata-icons-128x128");
            dlist.push_back("appdata-icons-128x128@2");
        }

        dlist.push_back(nullptr);
        h.set_opt(LRO_YUMDLIST, dlist.data());
    }
}

void RepoDownloader::common_handle_setup(LibrepoHandle & h, const DownloadData & download_data) {
    configure_handle_dlist(h, download_data.get_optional_metadata());
    h.set_opt(LRO_PRESERVETIME, static_cast<long>(download_data.preserve_remote_time));
    h.set_opt(LRO_REPOTYPE, LR_YUMREPO);
    h.set_opt(LRO_INTERRUPTIBLE, 1L);
    h.set_opt(LRO_GPGCHECK, download_data.config.get_repo_gpgcheck_option().get_value());
    h.set_opt(LRO_MAXMIRRORTRIES, static_cast<long>(download_data.max_mirror_tries));
    h.set_opt(LRO_MAXPARALLELDOWNLOADS, download_data.config.get_max_parallel_downloads_option().get_value());

    LrUrlVars * repomd_substs = nullptr;
    repomd_substs = lr_urlvars_set(repomd_substs, MD_FILENAME_GROUP_GZ, MD_FILENAME_GROUP);
    h.set_opt(LRO_YUMSLIST, repomd_substs);

    LrUrlVars * substs = nullptr;
    // Deprecated, we have direct access to base, therefore we don't need to set explicitly substitutions
    for (const auto & item : download_data.substitutions) {
        substs = lr_urlvars_set(substs, item.first.c_str(), item.second.c_str());
    }
    for (const auto & item : download_data.base->get_vars()->get_variables()) {
        substs = lr_urlvars_set(substs, item.first.c_str(), item.second.value.c_str());
    }
    h.set_opt(LRO_VARSUB, substs);

#ifdef LRO_SUPPORTS_CACHEDIR
    // If zchunk is enabled, set librepo cache dir
    if (download_data.config.get_main_config().get_zchunk_option().get_value()) {
        h.set_opt(LRO_CACHEDIR, download_data.config.get_basecachedir_option().get_value().c_str());
    }
#endif
}

void RepoDownloader::apply_http_headers(DownloadData & download_data, LibrepoHandle & handle) {
    if (download_data.http_headers.empty()) {
        handle.set_opt(LRO_HTTPHEADER, nullptr);
        return;
    }

    std::unique_ptr<char * [], std::function<void(char **)>> lr_headers {
        nullptr, [](char ** ptr) {
            for (auto item = ptr; *item != nullptr; ++item) {
                delete[] *item;
            }
            delete[] ptr;
        }
    };

    lr_headers.reset(new char * [download_data.http_headers.size() + 1] {});

    for (size_t i = 0; i < download_data.http_headers.size(); ++i) {
        const auto & header = download_data.http_headers[i];
        lr_headers[i] = new char[header.size() + 1];
        strcpy(lr_headers[i], header.c_str());
    }

    handle.set_opt(LRO_HTTPHEADER, lr_headers.get());
}

LibrepoResult RepoDownloader::perform(
    DownloadData & download_data, LibrepoHandle & handle, bool set_gpg_home_dir, CallbackData * cbd) {
    if (set_gpg_home_dir) {
        auto pubringdir = download_data.pgp.get_keyring_dir();
        handle.set_opt(LRO_GNUPGHOMEDIR, pubringdir.c_str());
    }

    add_countme_flag(download_data, handle);
    auto & config = download_data.config;

    // Callbacks are set only if callback data (cbd) is passed
    auto * download_callbacks = download_data.base->get_download_callbacks();
    if (cbd && download_callbacks) {
        cbd->user_cb_data = download_callbacks->add_new_download(
            download_data.user_data,
            !config.get_name_option().get_value().empty()
                ? config.get_name_option().get_value().c_str()
                : (!config.get_id().empty() ? config.get_id().c_str() : "unknown"),
            -1);
        cbd->prev_total_to_download = 0;
        cbd->prev_downloaded = 0;
        cbd->sum_prev_downloaded = 0;

        handle.set_opt(LRO_PROGRESSCB, static_cast<LrProgressCb>(progress_cb));
        handle.set_opt(LRO_PROGRESSDATA, cbd);
        handle.set_opt(LRO_FASTESTMIRRORCB, static_cast<LrFastestMirrorCb>(fastest_mirror_cb));
        handle.set_opt(LRO_FASTESTMIRRORDATA, &cbd);
        handle.set_opt(LRO_HMFCB, static_cast<LrHandleMirrorFailureCb>(mirror_failure_cb));
    }

    try {
        auto result = handle.perform();
        if (cbd && download_callbacks) {
            download_callbacks->end(cbd->user_cb_data, DownloadCallbacks::TransferStatus::SUCCESSFUL, nullptr);
        }
        return result;
    } catch (const LibrepoError & ex) {
        if (cbd && download_callbacks) {
            download_callbacks->end(cbd->user_cb_data, DownloadCallbacks::TransferStatus::ERROR, ex.what());
        }
        throw;
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
///
/// In the below comments, the window's current position will be referred to
/// as "this window" for brevity.
void RepoDownloader::add_countme_flag(DownloadData & download_data, LibrepoHandle & handle) {
    auto & logger = *(download_data.base)->get_logger();
    auto & config = download_data.config;

    // Bail out if not counting or not running as root (since the persistdir is
    // only root-writable)
    if (!config.get_countme_option().get_value() || getuid() != 0)
        return;

    // Bail out if not a remote handle
    long local;
    handle.get_info(LRI_LOCAL, &local);
    if (local)
        return;

    // Bail out if no metalink or mirrorlist is defined
    auto & metalink = config.get_metalink_option();
    auto & mirrorlist = config.get_mirrorlist_option();
    if ((metalink.empty() || metalink.get_value().empty()) && (mirrorlist.empty() || mirrorlist.get_value().empty()))
        return;

    if (!std::filesystem::is_directory(config.get_persistdir())) {
        std::filesystem::create_directories(config.get_persistdir());
    }

    // Load the cookie
    std::filesystem::path file_path(config.get_persistdir());
    file_path /= COUNTME_COOKIE;

    int ver = COUNTME_VERSION;    // file format version (for future use)
    time_t epoch = 0;             // position of first observed window
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

        // Compute the epoch from this system's epoch or, if unknown, declare
        // this window as the epoch (unless stored in the cookie previously).
        time_t sysepoch = get_system_epoch();
        if (sysepoch)
            epoch = sysepoch - ((sysepoch - COUNTME_OFFSET) % COUNTME_WINDOW);
        if (!epoch)
            epoch = win;

        // Window step (0 at epoch)
        int64_t step = (win - epoch) / COUNTME_WINDOW;

        // Compute the bucket we are in
        uint32_t i;
        for (i = 0; i < COUNTME_BUCKETS.size(); ++i)
            if (step < COUNTME_BUCKETS[i])
                break;
        uint32_t bucket = i + 1;  // Buckets are numbered from 1

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


/* Returns this system's installation time ("epoch") as a UNIX timestamp.
 *
 * Uses the machine-id(5) file's mtime as a good-enough source of truth.  This
 * file is typically tied to the system's installation or first boot where it's
 * populated by an installer tool or init system, respectively, and is never
 * changed afterwards.
 *
 * Some systems, such as containers that don't run an init system, may have the
 * file missing, empty or uninitialized, in which case this function returns 0.
 */
time_t RepoDownloader::get_system_epoch() {
    std::string filename = "/etc/machine-id";
    std::string id;
    struct stat st;

    if (stat(filename.c_str(), &st) != 0 || !st.st_size)
        return 0;
    std::ifstream(filename) >> id;
    if (id == "uninitialized")
        return 0;

    return st.st_mtime;
}


}  // namespace libdnf5::repo
