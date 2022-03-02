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
#include "utils/bgettext/bgettext-lib.h"

#include "libdnf/common/exception.hpp"
#include "libdnf/repo/repo.hpp"

#include <librepo/librepo.h>

#include <filesystem>


namespace std {

// TODO(lukash) share this deleter with repo code
template <>
struct default_delete<GError> {
    void operator()(GError * ptr) noexcept { g_error_free(ptr); }
};

template <>
struct default_delete<LrPackageTarget> {
    void operator()(LrPackageTarget * ptr) noexcept { lr_packagetarget_free(ptr); }
};

}  // namespace std


namespace libdnf::repo {

int PackageDownloadCallbacks::end([[maybe_unused]] TransferStatus status, [[maybe_unused]] const char * msg) {
    return 0;
}
int PackageDownloadCallbacks::progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) {
    return 0;
}
int PackageDownloadCallbacks::mirror_failure([[maybe_unused]] const char * msg, [[maybe_unused]] const char * url) {
    return 0;
}


static int end_callback(void * data, LrTransferStatus status, const char * msg) {
    if (!data) {
        return 0;
    }

    auto cb_status = static_cast<PackageDownloadCallbacks::TransferStatus>(status);
    return static_cast<PackageDownloadCallbacks *>(data)->end(cb_status, msg);
}

static int progress_callback(void * data, double total_to_download, double downloaded) {
    if (!data) {
        return 0;
    }

    return static_cast<PackageDownloadCallbacks *>(data)->progress(total_to_download, downloaded);
}

static int mirror_failure_callback(void * data, const char * msg, const char * url) {
    if (!data) {
        return 0;
    }

    return static_cast<PackageDownloadCallbacks *>(data)->mirror_failure(msg, url);
}

class PackageTarget {
public:
    PackageTarget(
        const libdnf::rpm::Package & package,
        const std::string & destination,
        std::unique_ptr<PackageDownloadCallbacks> && callbacks)
        : package(package),
          destination(destination),
          callbacks(std::move(callbacks)) {}

    libdnf::rpm::Package package;
    std::string destination;
    std::unique_ptr<PackageDownloadCallbacks> callbacks;
};


class PackageDownloader::Impl {
    friend PackageDownloader;
    std::vector<PackageTarget> targets;
};


PackageDownloader::PackageDownloader() : p_impl(std::make_unique<Impl>()) {}
PackageDownloader::~PackageDownloader() = default;


void PackageDownloader::add(
    const libdnf::rpm::Package & package, std::unique_ptr<PackageDownloadCallbacks> && callbacks) {
    add(package, std::filesystem::path(package.get_repo()->get_cachedir()) / "packages", std::move(callbacks));
}


void PackageDownloader::add(
    const libdnf::rpm::Package & package,
    const std::string & destination,
    std::unique_ptr<PackageDownloadCallbacks> && callbacks) {
    p_impl->targets.emplace_back(package, destination, std::move(callbacks));
}


void PackageDownloader::download(bool fail_fast, bool resume) try {
    GError * err{nullptr};
    GSList * list{nullptr};
    std::vector<std::unique_ptr<LrPackageTarget>> lr_targets;

    for (auto it = p_impl->targets.rbegin(); it != p_impl->targets.rend(); ++it) {
        std::filesystem::create_directory(it->destination);

        auto lr_target = lr_packagetarget_new_v3(
            it->package.get_repo()->downloader->get_cached_handle(),
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
