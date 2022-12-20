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

#ifndef LIBDNF_REPO_FILE_DOWNLOADER_HPP
#define LIBDNF_REPO_FILE_DOWNLOADER_HPP

#include "libdnf/common/exception.hpp"
#include "libdnf/conf/config_main.hpp"
#include "libdnf/repo/repo_weak.hpp"

#include <memory>

namespace libdnf::repo {

class FileDownloadError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::repo"; }
    const char * get_name() const noexcept override { return "FileDownloadError"; }
};


class FileDownloader {
public:
    explicit FileDownloader(const libdnf::BaseWeakPtr & base);
    ~FileDownloader();

    /// Adds a file (URL) to download.
    /// @param repo The repository whose settings are to be used.
    /// @param url The file (url) to download.
    /// @param destination The file path to which to download the file.
    void add(libdnf::repo::RepoWeakPtr & repo, const std::string & url, const std::string & destination);

    /// Adds a file (URL) to download. The settings from ConfigMain passed in the FileDownloader constructor are used.
    /// @param url The file (url) to download.
    /// @param destination The file path to which to download the file.
    void add(const std::string & url, const std::string & destination);

    /// Download the previously added files (URLs).
    /// @param fail_fast Whether to fail the whole download on a first error or keep downloading.
    /// @param resume Whether to try to resume the download if a destination package already exists.
    void download(bool fail_fast, bool resume);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
    BaseWeakPtr base;
};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_FILE_DOWNLOADER_HPP
