// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "utils.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <array>
#include <string>

namespace dnfdaemon {

bool write_to_fd(const std::string & message, int out_fd, std::string & error_msg) {
    const auto fd_buffer_size = static_cast<size_t>(fcntl(out_fd, F_GETPIPE_SZ));
    // TODO(mblaha): make the timeout configurable
    const int timeout = 30000;

    std::array<pollfd, 1> pfds{};
    pfds[0].fd = out_fd;
    pfds[0].events = POLLOUT;

    ssize_t total_bytes_written = 0;
    size_t bytes_remaining = message.size();

    int ready = -1;
    bool success = true;
    while (bytes_remaining > 0 && success) {
        ready = poll(pfds.data(), pfds.size(), timeout);
        switch (ready) {
            case -1:
                // poll call failed
                error_msg = "poll() call failed.";
                success = false;
                break;
            case 0:
                // timeout was reached
                error_msg = "Timeout reached.";
                success = false;
                break;
            default:
                if ((pfds[0].revents & POLLOUT) != 0) {
                    // file descriptor is ready for writing
                    const auto bytes_written =
                        write(out_fd, message.data() + total_bytes_written, std::min(fd_buffer_size, bytes_remaining));
                    if (bytes_written == -1) {
                        if ((errno != EAGAIN && errno != EWOULDBLOCK)) {
                            // error writing
                            // e.g. the client has closed the read end of the pipe
                            success = false;
                            error_msg = "Error writing to the file descriptor.";
                        }
                    } else {
                        total_bytes_written += bytes_written;
                        bytes_remaining -= static_cast<size_t>(bytes_written);
                    }
                } else {
                    success = false;
                    error_msg = "File descriptor not ready for writing.";
                }
        }
    }
    return success;
}

}  // namespace dnfdaemon
