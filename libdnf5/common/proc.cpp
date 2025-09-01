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

#include "libdnf5/common/proc.hpp"

#include <errno.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <unistd.h>

#include <cstdlib>

namespace libdnf5 {

uid_t read_login_uid_from_proc(pid_t pid) noexcept {
    auto in = open(fmt::format("/proc/{}/loginuid", pid).c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (in == -1) {
        return INVALID_UID;
    }

    ssize_t len;
    char buf[16];
    do {
        errno = 0;
        len = read(in, buf, sizeof(buf) - 1);
    } while (len < 0 && errno == EINTR);

    close(in);

    if (len <= 0) {
        return INVALID_UID;
    }

    buf[len] = '\0';
    char * endptr;
    errno = 0;
    auto uid = static_cast<uid_t>(strtol(buf, &endptr, 10));
    if (buf == endptr || errno != 0) {
        return INVALID_UID;
    }

    return uid;
}

uid_t get_login_uid() noexcept {
    static uid_t cached_uid = libdnf5::INVALID_UID;
    if (cached_uid == libdnf5::INVALID_UID) {
        cached_uid = libdnf5::read_login_uid_from_proc(getpid());
        if (cached_uid == libdnf5::INVALID_UID) {
            cached_uid = getuid();
        }
    }
    return cached_uid;
}

}  // namespace libdnf5
