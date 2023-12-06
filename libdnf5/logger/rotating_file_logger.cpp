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

#include "libdnf5/logger/rotating_file_logger.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <fcntl.h>
#include <fmt/format.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include <mutex>

namespace libdnf5 {

class RotatingFileLogger::Impl {
public:
    explicit Impl(const std::filesystem::path & base_file_path, std::size_t max_bytes, std::size_t backup_count);

    void write(const char * line) noexcept;

private:
    bool should_rotate(int fd, std::size_t msg_len) const noexcept;

    const std::filesystem::path base_file_path;
    const std::size_t max_bytes;
    const std::size_t backup_count;

    std::mutex stream_mutex;
};


RotatingFileLogger::Impl::Impl(
    const std::filesystem::path & base_file_path, std::size_t max_bytes, std::size_t backup_count)
    : base_file_path{base_file_path},
      max_bytes{max_bytes},
      backup_count{backup_count} {
    if (::open(
            base_file_path.c_str(), O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) ==
        -1) {
        throw FileSystemError(errno, base_file_path, M_("Cannot open log file"));
    }
}


void RotatingFileLogger::Impl::write(const char * line) noexcept {
    try {
        // required for thread safety
        std::lock_guard<std::mutex> guard(stream_mutex);

        auto line_len = strlen(line);
        while (true) {
            // open (create) the base log file and lock it
            auto fd = ::open(
                base_file_path.c_str(),
                O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd == -1) {
                return;
            }
            ::flock(fd, LOCK_EX);

            if (!should_rotate(fd, line_len)) {
                // no need to rotate log files, just write a message to the log and return
                std::size_t written = 0;
                ssize_t ret;
                do {
                    ret = ::write(fd, line + written, line_len - written);
                    if (ret <= 0) {
                        break;
                    }
                    written += static_cast<std::size_t>(ret);
                } while (written < line_len);
                ::close(fd);
                return;
            }

            // A rotation of the log files is needed, rotate the files and start from the beginning.
            // But before that, we will verify that another process did not perform the rotation.

            auto fd_base_file = ::open(base_file_path.c_str(), O_RDONLY | O_CLOEXEC);
            if (fd_base_file == -1) {
                // Unable to reopen the base log file, it was probably rotated by another process.
                // Close the locked file descriptor and start from the beginning.
                ::close(fd);
                continue;
            }
            if (::lseek(fd_base_file, 0, SEEK_END) != ::lseek(fd, 0, SEEK_END)) {
                // The size of the base log file is different than the size of the locked descriptor,
                // probably rotated by another process.
                // Close file descriptors and start from the beginning.
                ::close(fd_base_file);
                ::close(fd);
                continue;
            }
            ::close(fd_base_file);

            // A log file rotation is needed and so far no one has done it.
            // Let's rotate the files and start from the beginning.
            try {
                for (auto file_idx = backup_count; file_idx > 0; --file_idx) {
                    auto path_old = file_idx > 1 ? fmt::format("{}.{}", base_file_path.string(), file_idx - 1)
                                                 : base_file_path.string();
                    auto path_new = fmt::format("{}.{}", base_file_path.string(), file_idx);
                    ::rename(path_old.c_str(), path_new.c_str());
                }
            } catch (...) {
            }
            ::close(fd);
        }
    } catch (...) {
    }
}


bool RotatingFileLogger::Impl::should_rotate(int fd, std::size_t msg_len) const noexcept {
    if (max_bytes == 0 || backup_count < 1) {
        return false;
    }
    auto len = ::lseek(fd, 0, SEEK_END);
    if (len < 0) {
        // error
        return false;
    }
    if (len == 0) {
        // file is empty, rotation makes no sense
        return false;
    }
    return static_cast<std::size_t>(len) + msg_len > max_bytes;
}


RotatingFileLogger::RotatingFileLogger(
    const std::filesystem::path & base_file_path, std::size_t max_bytes, std::size_t backup_count)
    : p_impl(new Impl(base_file_path, max_bytes, backup_count)) {}


RotatingFileLogger::~RotatingFileLogger() = default;


void RotatingFileLogger::write(const char * line) noexcept {
    p_impl->write(line);
}

}  // namespace libdnf5
