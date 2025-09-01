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

#include "utils.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>

namespace libdnf5::utils::fs {

namespace stdfs = std::filesystem;

[[nodiscard]] bool have_files_same_content_noexcept(const char * file_path1, const char * file_path2) noexcept {
    static constexpr int block_size = 4096;
    bool ret = false;
    int fd1 = -1;
    int fd2 = -1;
    do {
        if ((fd1 = open(file_path1, O_CLOEXEC)) == -1) {
            break;
        }
        if ((fd2 = open(file_path2, O_CLOEXEC)) == -1) {
            break;
        }
        auto len1 = lseek(fd1, 0, SEEK_END);
        auto len2 = lseek(fd2, 0, SEEK_END);
        if (len1 != len2) {
            break;
        }
        ret = true;
        if (len1 == 0) {
            break;
        }
        lseek(fd1, 0, SEEK_SET);
        lseek(fd2, 0, SEEK_SET);
        char buf1[block_size];
        char buf2[block_size];
        ssize_t readed;
        do {
            readed = read(fd1, &buf1, block_size);
            auto readed2 = read(fd2, &buf2, block_size);
            if (readed2 != readed || std::memcmp(&buf1, &buf2, static_cast<size_t>(readed)) != 0) {
                ret = false;
                break;
            }
        } while (readed == block_size);
    } while (false);

    if (fd1 != -1) {
        close(fd1);
    }
    if (fd2 != -1) {
        close(fd2);
    }
    return ret;
}


void move_recursive(const std::filesystem::path & src, const std::filesystem::path & dest) {
    try {
        std::filesystem::rename(src, dest);
    } catch (const std::filesystem::filesystem_error &) {
        std::filesystem::copy(src, dest, std::filesystem::copy_options::recursive);
        std::filesystem::remove_all(src);
    }
}

[[nodiscard]] std::vector<std::filesystem::path> create_sorted_file_list(
    const std::vector<std::filesystem::path> & directories, std::string_view file_extension) {
    std::vector<stdfs::path> paths;

    for (const auto & dir : directories) {
        std::error_code ec;
        for (const auto & dentry : stdfs::directory_iterator(dir, ec)) {
            const auto & path = dentry.path();
            if (dentry.is_regular_file() && path.extension() == file_extension) {
                const auto & path_fname = path.filename();
                bool found{false};
                for (const auto & path_in_list : paths) {
                    if (path_fname == path_in_list.filename()) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    paths.push_back(path);
                }
            }
        }
    }

    // sort all drop-in configuration files alphabetically by their names
    std::sort(paths.begin(), paths.end(), [](const stdfs::path & p1, const stdfs::path & p2) {
        return p1.filename() < p2.filename();
    });

    return paths;
}

}  // namespace libdnf5::utils::fs
