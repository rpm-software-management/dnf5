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

#include "libdnf5/repo/package_downloader.hpp"

#include "repo_downloader.hpp"
#include "temp_files_memory.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/repo/download_callbacks.hpp"
#include "libdnf5/repo/repo.hpp"
#include "libdnf5/repo/repo_errors.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <fmt/format.h>
#include <librepo/librepo.h>

#include <algorithm>
#include <filesystem>


namespace std {

template <>
struct default_delete<LrPackageTarget> {
    void operator()(LrPackageTarget * ptr) noexcept { lr_packagetarget_free(ptr); }
};

}  // namespace std


namespace libdnf5::repo {

class PackageTarget {
public:
    PackageTarget(const libdnf5::rpm::Package & package, const std::string & destination, void * user_data)
        : package(package),
          destination(destination),
          user_data(user_data) {}

    ~PackageTarget() {
        if (need_call_end_callback) {
            if (auto * download_callbacks = package.get_base()->get_download_callbacks()) {
                download_callbacks->end(user_cb_data, DownloadCallbacks::TransferStatus::ERROR, "Interrupted");
            }
        }
    }

    libdnf5::rpm::Package package;
    std::string destination;
    void * user_data;
    void * user_cb_data{nullptr};
    bool need_call_end_callback{false};
};

static int end_callback(void * data, LrTransferStatus status, const char * msg) {
    libdnf_assert(data != nullptr, "data in callback must be set");

    auto * package_target = static_cast<PackageTarget *>(data);
    auto cb_status = static_cast<DownloadCallbacks::TransferStatus>(status);
    if (auto * download_callbacks = package_target->package.get_base()->get_download_callbacks()) {
        libdnf_assert(package_target->need_call_end_callback == true, "unexpected end_callback call");
        package_target->need_call_end_callback = false;
        return download_callbacks->end(package_target->user_cb_data, cb_status, msg);
    }
    return 0;
}

static int progress_callback(void * data, double total_to_download, double downloaded) {
    libdnf_assert(data != nullptr, "data in callback must be set");

    auto * package_target = static_cast<PackageTarget *>(data);
    if (auto * download_callbacks = package_target->package.get_base()->get_download_callbacks()) {
        return download_callbacks->progress(package_target->user_cb_data, total_to_download, downloaded);
    }
    return 0;
}

static int mirror_failure_callback(void * data, const char * msg, const char * url) {
    libdnf_assert(data != nullptr, "data in callback must be set");

    auto * package_target = static_cast<PackageTarget *>(data);
    if (auto * download_callbacks = package_target->package.get_base()->get_download_callbacks()) {
        return download_callbacks->mirror_failure(package_target->user_cb_data, msg, url, nullptr);
    }
    return 0;
}


class PackageDownloader::Impl {
public:
    Impl(const BaseWeakPtr & base) : base(base), fail_fast(true), resume(true) {}

private:
    friend PackageDownloader;

    BaseWeakPtr base;

    std::vector<PackageTarget> targets;
    std::optional<bool> keep_packages;
    bool fail_fast;
    bool resume;
};


PackageDownloader::PackageDownloader(const BaseWeakPtr & base) : p_impl(std::make_unique<Impl>(base)) {}
PackageDownloader::PackageDownloader(Base & base) : p_impl(std::make_unique<Impl>(base.get_weak_ptr())) {}
PackageDownloader::~PackageDownloader() = default;


void PackageDownloader::add(const libdnf5::rpm::Package & package, void * user_data) {
    add(package, package.get_repo()->get_packages_download_dir(), user_data);
}


void PackageDownloader::add(const libdnf5::rpm::Package & package, const std::string & destination, void * user_data) {
    p_impl->targets.emplace_back(package, destination, user_data);
}


void PackageDownloader::download() try {
    if (p_impl->targets.empty()) {
        return;
    }

    auto & config = p_impl->base->get_config();
    auto use_cache_only = config.get_cacheonly_option().get_value() == "all";
    GError * err{nullptr};

    std::vector<std::unique_ptr<LrPackageTarget>> lr_targets;
    lr_targets.reserve(p_impl->targets.size());
    for (auto & pkg_target : p_impl->targets) {
        if (use_cache_only && !pkg_target.package.is_available_locally()) {
            throw RepoCacheonlyError(
                M_("Cannot download the \"{0}\" package, cacheonly option is activated."),
                pkg_target.package.get_nevra());
        }

        std::filesystem::create_directories(pkg_target.destination);

        if (auto * download_callbacks = pkg_target.package.get_base()->get_download_callbacks()) {
            pkg_target.user_cb_data = download_callbacks->add_new_download(
                pkg_target.user_data,
                pkg_target.package.get_full_nevra().c_str(),
                static_cast<double>(pkg_target.package.get_download_size()));
            pkg_target.need_call_end_callback = true;
        }

        if (pkg_target.package.is_available_locally()) {
            // Copy local packages to their destination directories
            std::filesystem::path source = pkg_target.package.get_package_path();
            std::filesystem::path destination = pkg_target.destination / source.filename();
            std::error_code ec;
            const bool same_file = std::filesystem::equivalent(source, destination, ec);
            if (!same_file) {
                std::filesystem::copy(source, destination, std::filesystem::copy_options::overwrite_existing, ec);
            }
            if (auto * download_callbacks = pkg_target.package.get_base()->get_download_callbacks()) {
                std::string msg;
                DownloadCallbacks::TransferStatus status;
                if (ec) {
                    status = DownloadCallbacks::TransferStatus::ERROR;
                    msg = ec.message();
                } else {
                    status = DownloadCallbacks::TransferStatus::ALREADYEXISTS;
                    if (same_file) {
                        msg = "Already downloaded";
                    } else {
                        msg = fmt::format("Copied from {}", source.string());
                    }
                }
                pkg_target.need_call_end_callback = false;
                download_callbacks->end(pkg_target.user_cb_data, status, msg.c_str());
            }
            continue;
        }

        auto * lr_target = lr_packagetarget_new_v3(
            pkg_target.package.get_repo()->get_downloader().get_cached_handle().get(),
            pkg_target.package.get_location().c_str(),
            pkg_target.destination.c_str(),
            static_cast<LrChecksumType>(pkg_target.package.get_checksum().get_type()),
            pkg_target.package.get_checksum().get_checksum().c_str(),
            static_cast<int64_t>(pkg_target.package.get_download_size()),
            pkg_target.package.get_baseurl().empty() ? nullptr : pkg_target.package.get_baseurl().c_str(),
            p_impl->resume,
            progress_callback,
            &pkg_target,
            end_callback,
            mirror_failure_callback,
            0,
            0,
            &err);

        if (!lr_target) {
            throw LibrepoError(std::unique_ptr<GError>(err));
        }

        lr_targets.emplace_back(lr_target);
    }

    // Adding items to the end of GSList is slow. We go from the back and add items to the beginning.
    GSList * list{nullptr};
    for (auto it = lr_targets.rbegin(); it != lr_targets.rend(); ++it) {
        list = g_slist_prepend(list, it->get());
    }
    std::unique_ptr<GSList, decltype(&g_slist_free)> list_holder(list, &g_slist_free);

    LrPackageDownloadFlag flags = static_cast<LrPackageDownloadFlag>(0);
    if (p_impl->fail_fast) {
        flags = static_cast<LrPackageDownloadFlag>(flags | LR_PACKAGEDOWNLOAD_FAILFAST);
    }

    // Store file paths of packages we don't want to keep cached.
    auto removal_configured = !config.get_keepcache_option().get_value();
    auto removal_enforced = p_impl->keep_packages.has_value() && !p_impl->keep_packages.value();
    auto keep_enforced = p_impl->keep_packages.has_value() && p_impl->keep_packages.value();
    if (removal_enforced || (!keep_enforced && removal_configured)) {
        std::vector<std::string> package_paths;
        std::transform(
            p_impl->targets.begin(),
            p_impl->targets.end(),
            std::back_inserter(package_paths),
            [](const PackageTarget & target) {
                return std::filesystem::canonical(std::filesystem::path(target.destination)) /
                       std::filesystem::path(target.package.get_location()).filename();
            });

        auto & cachedir = config.get_cachedir_option().get_value();
        TempFilesMemory temp_files_memory(p_impl->base, cachedir);
        temp_files_memory.add_files(package_paths);
    }

    if (!lr_download_packages(list, flags, &err)) {
        throw LibrepoError(std::unique_ptr<GError>(err));
    }
} catch (const RepoCacheonlyError & e) {
    throw;
} catch (const std::runtime_error & e) {
    throw_with_nested(PackageDownloadError(M_("Failed to download packages")));
}

void PackageDownloader::set_fail_fast(bool value) {
    p_impl->fail_fast = value;
}

void PackageDownloader::set_resume(bool value) {
    p_impl->resume = value;
}

void PackageDownloader::force_keep_packages(bool value) {
    p_impl->keep_packages = value;
}

}  // namespace libdnf5::repo
