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
#include "libdnf/repo/download_callbacks.hpp"
#include "libdnf/rpm/package.hpp"

#include <memory>

namespace libdnf::repo {

class PackageDownloadError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::repo"; }
    const char * get_name() const noexcept override { return "PackageDownloadError"; }
};


class PackageDownloader {
public:
    PackageDownloader();
    ~PackageDownloader();

    /// Adds a package to download to the standard location of repo cachedir/packages.
    /// @param package The package to download.
    /// @param callbacks (optional) A pointer to an instance of the `PackageDownloadCallbacks` class.
    void add(const libdnf::rpm::Package & package, std::unique_ptr<DownloadCallbacks> && callbacks = {});

    /// Adds a package to download to a specific destination directory.
    /// @param package The package to download.
    /// @param destination The directory to which to download the package.
    /// @param callbacks (optional) A pointer to an instance of the `PackageDownloadCallbacks` class.
    void add(
        const libdnf::rpm::Package & package,
        const std::string & destination,
        std::unique_ptr<DownloadCallbacks> && callbacks = {});

    /// Download the previously added packages.
    /// @param fail_fast Whether to fail the whole download on a first error or keep downloading.
    /// @param resume Whether to try to resume the download if a destination package already exists.
    void download(bool fail_fast, bool resume);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
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
