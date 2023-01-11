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

#include "libdnf/repo/package_downloader.hpp"

#include "repo_downloader.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/base/base.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/repo/download_callbacks.hpp"
#include "libdnf/repo/repo.hpp"

#include <librepo/librepo.h>

#include <filesystem>


namespace std {

template <>
struct default_delete<LrPackageTarget> {
    void operator()(LrPackageTarget * ptr) noexcept { lr_packagetarget_free(ptr); }
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

class PackageTarget {
public:
    PackageTarget(
        const libdnf::rpm::Package & package,
        const std::string & destination,
        std::unique_ptr<DownloadCallbacks> && callbacks)
        : package(package),
          destination(destination),
          callbacks(std::move(callbacks)) {}

    libdnf::rpm::Package package;
    std::string destination;
    std::unique_ptr<DownloadCallbacks> callbacks;
};


class PackageDownloader::Impl {
    friend PackageDownloader;
    std::vector<PackageTarget> targets;
};


PackageDownloader::PackageDownloader(const BaseWeakPtr & base) : p_impl(std::make_unique<Impl>()), base(base) {}
PackageDownloader::~PackageDownloader() = default;


void PackageDownloader::add(const libdnf::rpm::Package & package) {
    add(package, std::filesystem::path(package.get_repo()->get_cachedir()) / "packages");
}


void PackageDownloader::add(const libdnf::rpm::Package & package, const std::string & destination) {
    p_impl->targets.emplace_back(package, destination, base->create_download_callbacks(package));
}


void PackageDownloader::download(bool fail_fast, bool resume) try {
    GError * err{nullptr};
    GSList * list{nullptr};
    std::vector<std::unique_ptr<LrPackageTarget>> lr_targets;

    for (auto it = p_impl->targets.rbegin(); it != p_impl->targets.rend(); ++it) {
        std::filesystem::create_directory(it->destination);

        auto lr_target = lr_packagetarget_new_v3(
            it->package.get_repo()->downloader->get_cached_handle().get(),
            it->package.get_location().c_str(),
            it->destination.c_str(),
            static_cast<LrChecksumType>(it->package.get_checksum().get_type()),
            it->package.get_checksum().get_checksum().c_str(),
            static_cast<int64_t>(it->package.get_package_size()),
            it->package.get_baseurl().empty() ? nullptr : it->package.get_baseurl().c_str(),
            resume,
            progress_callback,
            it->callbacks.get(),
            end_callback,
            mirror_failure_callback,
            0,
            0,
            &err);

        if (lr_target == nullptr) {
            throw LibrepoError(std::unique_ptr<GError>(err));
        }

        lr_targets.emplace_back(lr_target);
        list = g_slist_prepend(list, lr_target);
    }

    std::unique_ptr<GSList, decltype(&g_slist_free)> list_holder(list, &g_slist_free);

    LrPackageDownloadFlag flags = static_cast<LrPackageDownloadFlag>(0);
    if (fail_fast) {
        flags = static_cast<LrPackageDownloadFlag>(flags | LR_PACKAGEDOWNLOAD_FAILFAST);
    }

    if (!lr_download_packages(list, flags, &err)) {
        throw LibrepoError(std::unique_ptr<GError>(err));
    }
} catch (const std::runtime_error & e) {
    throw_with_nested(PackageDownloadError(M_("Failed to download packages")));
}

}  // namespace libdnf::repo
