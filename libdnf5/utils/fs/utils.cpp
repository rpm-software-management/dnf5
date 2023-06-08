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

#include "utils.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>

namespace libdnf5::utils::fs {


bool have_files_same_content_noexcept(const char * file_path1, const char * file_path2) noexcept {
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

}  // namespace libdnf5::utils::fs
