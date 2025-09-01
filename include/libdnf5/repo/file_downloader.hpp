// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_REPO_FILE_DOWNLOADER_HPP
#define LIBDNF5_REPO_FILE_DOWNLOADER_HPP

#include "file_downloader_errors.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/conf/config_main.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/repo/repo_weak.hpp"

#include <memory>

namespace libdnf5::repo {

class LIBDNF_API FileDownloader {
public:
    explicit FileDownloader(const libdnf5::BaseWeakPtr & base);
    explicit FileDownloader(libdnf5::Base & base);
    ~FileDownloader();

    /// Adds a file (URL) to download.
    /// @param repo The repository whose settings are to be used.
    /// @param url The file (url) to download.
    /// @param destination The file path to which to download the file.
    /// @param user_data User data.
    void add(
        libdnf5::repo::RepoWeakPtr & repo,
        const std::string & url,
        const std::string & destination,
        void * user_data = nullptr);

    /// Adds a file (URL) to download. The settings from ConfigMain passed in the FileDownloader constructor are used.
    /// @param url The file (url) to download.
    /// @param destination The file path to which to download the file.
    /// @param user_data User data.
    void add(const std::string & url, const std::string & destination, void * user_data = nullptr);

    /// Download the previously added files (URLs).
    void download();

    /// Configure whether to fail the whole download on a first error or keep downloading.
    /// @param value If true, download will fail on the first error, otherwise it continues.
    /// This is set to true by default.
    void set_fail_fast(bool value);

    /// Configure whether to try resuming the download if a destination package already exists.
    /// @param value If true, download is tried to be resumed, otherwise it starts over.
    /// This is set to true by default.
    void set_resume(bool value);

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_FILE_DOWNLOADER_HPP
