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

#ifndef LIBDNF_REPO_PACKAGE_DOWNLOADER_HPP
#define LIBDNF_REPO_PACKAGE_DOWNLOADER_HPP

#include "libdnf/conf/config_main.hpp"
#include "libdnf/rpm/package.hpp"

#include <memory>
#include <optional>

namespace libdnf::repo {

class PackageDownloadError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::repo"; }
    const char * get_name() const noexcept override { return "PackageDownloadError"; }
};


class PackageDownloader {
public:
    explicit PackageDownloader(const libdnf::BaseWeakPtr & base);
    explicit PackageDownloader(libdnf::Base & base);
    ~PackageDownloader();

    /// Adds a package to download to the standard location of repo cachedir/packages.
    /// @param package The package to download.
    /// @param user_data User data.
    void add(const libdnf::rpm::Package & package, void * user_data = nullptr);

    /// Adds a package to download to a specific destination directory.
    /// @param package The package to download.
    /// @param destination The directory to which to download the package.
    /// @param user_data User data.
    void add(const libdnf::rpm::Package & package, const std::string & destination, void * user_data = nullptr);

    /// Download the previously added packages.
    /// @param fail_fast Whether to fail the whole download on a first error or keep downloading.
    /// @param resume Whether to try to resume the download if a destination package already exists.
    void download(bool fail_fast, bool resume);

    /// Explicitly setup the behavior related to packages caching.
    /// By default, the `keepcache` configuration option is used to determine whether to keep
    /// downloaded packages even after following successful transaction.
    ///
    /// @param value If true, packages will be kept on the disk after downloading
    /// regardless the `keepcache` option value, if false, it enforces packages removal after
    /// the next successful transaction.
    void force_keep_packages(bool value) { keep_packages = value; }

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
    std::optional<bool> keep_packages;
};

/// Wraps librepo PackageTarget
// TODO(lukash) what to do with the ChecksumType enum? It's not actually used anywhere.
//struct PackageTarget {
//public:
//    // Enum of supported checksum types.
//    // NOTE! This enum guarantee to be sorted by "hash quality"
//    // TODO(jrohel): Use the same enum/checksum utilities on all places.
//    enum class ChecksumType {
//        UNKNOWN,
//        MD5,     //    The most weakest hash
//        SHA1,    //  |
//        SHA224,  //  |
//        SHA256,  //  |
//        SHA384,  // \|/
//        SHA512,  //    The most secure hash
//    };
//
//};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_PACKAGE_DOWNLOADER_HPP
