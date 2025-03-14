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

#include "libdnf5/repo/file_downloader.hpp"

#include "repo_downloader.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/repo/download_callbacks.hpp"
#include "libdnf5/repo/repo.hpp"
#include "libdnf5/repo/repo_errors.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <librepo/librepo.h>

namespace std {

template <>
struct default_delete<LrDownloadTarget> {
    void operator()(LrDownloadTarget * ptr) noexcept { lr_downloadtarget_free(ptr); }
};

}  // namespace std


namespace libdnf5::repo {

class FileTarget {
public:
    FileTarget(
        BaseWeakPtr & base,
        RepoWeakPtr & repo,
        const std::string & url,
        const std::string & destination,
        void * user_data)
        : base(base),
          repo(repo),
          url(url),
          destination(destination),
          user_data(user_data) {}

    FileTarget(BaseWeakPtr & base, const std::string & url, const std::string & destination, void * user_data)
        : base(base),
          url(url),
          destination(destination),
          user_data(user_data) {}

    ~FileTarget() {
        if (need_call_end_callback) {
            if (auto * download_callbacks = base->get_download_callbacks()) {
                download_callbacks->end(user_cb_data, DownloadCallbacks::TransferStatus::ERROR, "Interrupted");
            }
        }
    }

    BaseWeakPtr base;
    RepoWeakPtr repo;
    std::string url;
    std::string destination;
    void * user_data;
    void * user_cb_data{nullptr};
    bool need_call_end_callback{false};
};


static int end_callback(void * data, LrTransferStatus status, const char * msg) {
    libdnf_assert(data != nullptr, "data in callback must be set");

    auto * file_target = static_cast<FileTarget *>(data);
    auto cb_status = static_cast<DownloadCallbacks::TransferStatus>(status);
    if (auto * download_callbacks = file_target->base->get_download_callbacks()) {
        libdnf_assert(file_target->need_call_end_callback == true, "unexpected end_callback call");
        file_target->need_call_end_callback = false;
        return download_callbacks->end(file_target->user_cb_data, cb_status, msg);
    }
    return 0;
}

static int progress_callback(void * data, double total_to_download, double downloaded) {
    libdnf_assert(data != nullptr, "data in callback must be set");

    auto * file_target = static_cast<FileTarget *>(data);
    if (auto * download_callbacks = file_target->base->get_download_callbacks()) {
        return download_callbacks->progress(file_target->user_cb_data, total_to_download, downloaded);
    }
    return 0;
}

static int mirror_failure_callback(void * data, const char * msg, const char * url) {
    libdnf_assert(data != nullptr, "data in callback must be set");

    auto * file_target = static_cast<FileTarget *>(data);
    if (auto * download_callbacks = file_target->base->get_download_callbacks()) {
        return download_callbacks->mirror_failure(file_target->user_cb_data, msg, url, nullptr);
    }
    return 0;
}


class FileDownloader::Impl {
public:
    Impl(const BaseWeakPtr & base) : base(base), fail_fast(true), resume(true) {}

private:
    friend FileDownloader;

    BaseWeakPtr base;

    std::vector<FileTarget> targets;
    bool fail_fast;
    bool resume;
};


FileDownloader::FileDownloader(const BaseWeakPtr & base) : p_impl(std::make_unique<Impl>(base)) {}

FileDownloader::FileDownloader(Base & base) : p_impl(std::make_unique<Impl>(base.get_weak_ptr())) {}

FileDownloader::~FileDownloader() = default;

void FileDownloader::add(
    RepoWeakPtr & repo, const std::string & url, const std::string & destination, void * user_data) {
    p_impl->targets.emplace_back(p_impl->base, repo, url, destination, user_data);
}

void FileDownloader::add(const std::string & url, const std::string & destination, void * user_data) {
    p_impl->targets.emplace_back(p_impl->base, url, destination, user_data);
}

void FileDownloader::download() try {
    if (p_impl->targets.empty()) {
        return;
    }

    auto & config = p_impl->base->get_config();
    auto & cacheonly = config.get_cacheonly_option().get_value();
    if (cacheonly == "all") {
        throw RepoCacheonlyError(M_("Cannot download files, cacheonly option is activated."));
    }

    std::vector<std::unique_ptr<LrDownloadTarget>> lr_targets;
    lr_targets.reserve(p_impl->targets.size());

    LibrepoHandle local_handle;
    local_handle.init_remote(config);

    auto * download_callbacks = p_impl->base->get_download_callbacks();

    for (auto & file_target : p_impl->targets) {
        LrHandle * handle;
        if (file_target.repo.is_valid()) {
            handle = file_target.repo->get_downloader().get_cached_handle().get();
        } else {
            handle = local_handle.get();
        }

        if (download_callbacks) {
            file_target.user_cb_data =
                download_callbacks->add_new_download(file_target.user_data, file_target.url.c_str(), -1);
            file_target.need_call_end_callback = true;
        }

        auto lr_target = lr_downloadtarget_new(
            handle,
            file_target.url.c_str(),
            NULL,
            -1,
            file_target.destination.c_str(),
            NULL,
            0,
            p_impl->resume,
            progress_callback,
            &file_target,
            end_callback,
            mirror_failure_callback,
            NULL,
            0,
            0,
            NULL,
            FALSE,
            FALSE);

        lr_targets.emplace_back(lr_target);
    }

    // Adding items to the end of GSList is slow. We go from the back and add items to the beginning.
    GSList * list{nullptr};
    for (auto it = lr_targets.rbegin(); it != lr_targets.rend(); ++it) {
        list = g_slist_prepend(list, it->get());
    }
    std::unique_ptr<GSList, decltype(&g_slist_free)> list_holder(list, &g_slist_free);

    GError * err{nullptr};
    if (!lr_download(list, p_impl->fail_fast, &err)) {
        throw LibrepoError(std::unique_ptr<GError>(err));
    }
} catch (const RepoCacheonlyError & e) {
    throw;
} catch (const std::runtime_error & e) {
    libdnf5::throw_with_nested(FileDownloadError(M_("Failed to download files")));
}

void FileDownloader::set_fail_fast(bool value) {
    p_impl->fail_fast = value;
}

void FileDownloader::set_resume(bool value) {
    p_impl->resume = value;
}

}  // namespace libdnf5::repo
