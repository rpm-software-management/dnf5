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

#include "libdnf/repo/file_downloader.hpp"

#include "repo_downloader.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/repo/repo.hpp"

#include <librepo/librepo.h>

namespace std {

template <>
struct default_delete<LrDownloadTarget> {
    void operator()(LrDownloadTarget * ptr) noexcept { lr_downloadtarget_free(ptr); }
};

}  // namespace std


namespace libdnf::repo {

static int end_callback(void * data, LrTransferStatus status, const char * msg) {
    if (!data) {
        return 0;
    }

    auto cb_status = static_cast<DownloadCallbacks::TransferStatus>(status);
    return static_cast<DownloadCallbacks *>(data)->end(cb_status, msg);
}

static int progress_callback(void * data, double total_to_download, double downloaded) {
    if (!data) {
        return 0;
    }

    return static_cast<DownloadCallbacks *>(data)->progress(total_to_download, downloaded);
}

static int mirror_failure_callback(void * data, const char * msg, const char * url) {
    if (!data) {
        return 0;
    }

    return static_cast<DownloadCallbacks *>(data)->mirror_failure(msg, url);
}

class FileTarget {
public:
    FileTarget(
        RepoWeakPtr & repo,
        const std::string & url,
        const std::string & destination,
        std::unique_ptr<DownloadCallbacks> && callbacks)
        : repo(repo),
          url(url),
          destination(destination),
          callbacks(std::move(callbacks)) {}

    FileTarget(
        const std::string & url, const std::string & destination, std::unique_ptr<DownloadCallbacks> && callbacks)
        : url(url),
          destination(destination),
          callbacks(std::move(callbacks)) {}

    RepoWeakPtr repo;
    std::string url;
    std::string destination;
    std::unique_ptr<DownloadCallbacks> callbacks;
};


class FileDownloader::Impl {
public:
    Impl(ConfigMain & config) : config(&config) {}
    ConfigMain * config;
    std::vector<FileTarget> targets;
};


FileDownloader::FileDownloader(ConfigMain & config) : p_impl(std::make_unique<Impl>(config)) {}
FileDownloader::~FileDownloader() = default;

void FileDownloader::add(
    RepoWeakPtr & repo,
    const std::string & url,
    const std::string & destination,
    std::unique_ptr<DownloadCallbacks> && callbacks) {
    p_impl->targets.emplace_back(repo, url, destination, std::move(callbacks));
}

void FileDownloader::add(
    const std::string & url, const std::string & destination, std::unique_ptr<DownloadCallbacks> && callbacks) {
    p_impl->targets.emplace_back(url, destination, std::move(callbacks));
}

void FileDownloader::download(bool fail_fast, bool resume) try {
    GSList * list{nullptr};
    std::vector<std::unique_ptr<LrDownloadTarget>> lr_targets;

    LibrepoHandle local_handle;
    local_handle.init_remote(*p_impl->config);

    for (auto it = p_impl->targets.rbegin(); it != p_impl->targets.rend(); ++it) {
        LrHandle * handle;
        if (it->repo.is_valid()) {
            handle = it->repo->downloader->get_cached_handle().get();
        } else {
            handle = local_handle.get();
        }

        auto lr_target = lr_downloadtarget_new(
            handle,
            it->url.c_str(),
            NULL,
            -1,
            it->destination.c_str(),
            NULL,
            0,
            resume,
            progress_callback,
            it->callbacks.get(),
            end_callback,
            mirror_failure_callback,
            NULL,
            0,
            0,
            NULL,
            FALSE,
            FALSE);

        lr_targets.emplace_back(lr_target);
        list = g_slist_prepend(list, lr_target);
    }

    std::unique_ptr<GSList, decltype(&g_slist_free)> list_holder(list, &g_slist_free);

    GError * err{nullptr};
    if (!lr_download(list, fail_fast, &err)) {
        throw LibrepoError(std::unique_ptr<GError>(err));
    }
} catch (const std::runtime_error & e) {
    throw_with_nested(FileDownloadError(M_("Failed to download files")));
}

}  // namespace libdnf::repo
