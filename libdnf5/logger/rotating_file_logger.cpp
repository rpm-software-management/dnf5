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
#include <sys/stat.h>
#include <unistd.h>

#include <mutex>

namespace libdnf5 {

const int LOG_FILE_OPEN_FLAGS = O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC;
const mode_t LOG_FILE_OPEN_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

class RotatingFileLogger::Impl {
public:
    explicit Impl(const std::filesystem::path & base_file_path, std::size_t max_bytes, std::size_t backup_count);
    ~Impl();

    void write(const char * line) noexcept;

private:
    bool should_rotate(std::size_t msg_len) const noexcept;

    const std::filesystem::path base_file_path;
    const std::size_t max_bytes;
    const std::size_t backup_count;

    std::mutex stream_mutex;

    int log_file_fd{-1};
};


RotatingFileLogger::Impl::Impl(
    const std::filesystem::path & base_file_path, std::size_t max_bytes, std::size_t backup_count)
    : base_file_path{base_file_path},
      max_bytes{max_bytes},
      backup_count{backup_count},
      log_file_fd(::open(base_file_path.c_str(), LOG_FILE_OPEN_FLAGS, LOG_FILE_OPEN_MODE)) {
    if (log_file_fd == -1) {
        throw FileSystemError(errno, base_file_path, M_("Cannot open log file"));
    }
}


RotatingFileLogger::Impl::~Impl() {
    if (log_file_fd != -1) {
        ::close(log_file_fd);
    }
}

void RotatingFileLogger::Impl::write(const char * line) noexcept {
    try {
        // required for thread safety
        std::lock_guard<std::mutex> guard(stream_mutex);

        auto line_len = strlen(line);
        while (true) {
            if (log_file_fd == -1) {
                // something is terribly wrong, cannot log
                return;
            }

            // acquire log file lock
            ::flock(log_file_fd, LOCK_EX);

            // verify that the log file has not been rotated by another process
            auto check_file_fd = ::open(base_file_path.c_str(), O_RDONLY | O_CLOEXEC);
            if (check_file_fd == -1) {
                // try to create log file in case it was rotated by another process
                auto log_file_fd_tmp = ::open(base_file_path.c_str(), LOG_FILE_OPEN_FLAGS, LOG_FILE_OPEN_MODE);
                if (log_file_fd_tmp != -1) {
                    // log file created, update log_file_fd and retry
                    ::close(log_file_fd);
                    log_file_fd = log_file_fd_tmp;
                    continue;
                }
                // cannot create the log file - this can mean that libdnf5 is
                // inside the rpm transaction which enters the chroot changing
                // path resolution. In such case just use log_file_fd without
                // checking rotation.
            } else {
                // check that log_file_fd and check_file_fd belong to the same inode
                struct stat log_fd_stat;
                struct stat check_fd_stat;
                if (::fstat(log_file_fd, &log_fd_stat) == -1 || fstat(check_file_fd, &check_fd_stat) == -1) {
                    // something went wrong, cannot stat log file.
                    // Close file descriptors and return
                    ::close(check_file_fd);
                    ::close(log_file_fd);
                    log_file_fd = -1;
                    return;
                }
                ::close(check_file_fd);
                if (log_fd_stat.st_ino != check_fd_stat.st_ino) {
                    // log file descriptor belongs to another file than base_file_path.
                    // Probably the log file was rotated by another process.
                    // Re-open log_file_fd and try again
                    ::close(log_file_fd);
                    log_file_fd = ::open(base_file_path.c_str(), LOG_FILE_OPEN_FLAGS, LOG_FILE_OPEN_MODE);
                    continue;
                }
                if (should_rotate(line_len)) {
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
                    ::close(log_file_fd);
                    log_file_fd = ::open(base_file_path.c_str(), LOG_FILE_OPEN_FLAGS, LOG_FILE_OPEN_MODE);
                    continue;
                }
            }

            // write the log line
            std::size_t written = 0;
            ssize_t ret;
            do {
                ret = ::write(log_file_fd, line + written, line_len - written);
                if (ret <= 0) {
                    break;
                }
                written += static_cast<std::size_t>(ret);
            } while (written < line_len);

            // we are done, unlock the log_file_fd and return
            ::flock(log_file_fd, LOCK_UN);
            return;
        }
    } catch (...) {
    }
}


bool RotatingFileLogger::Impl::should_rotate(std::size_t msg_len) const noexcept {
    if (max_bytes == 0 || backup_count < 1) {
        return false;
    }
    auto len = ::lseek(log_file_fd, 0, SEEK_END);
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
