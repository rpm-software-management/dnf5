// Copyright Contributors to the DNF5 project.
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


#include "libdnf5/utils/locker.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace libdnf5::utils {

Locker::Locker(const std::string & path) : path(path) {};

bool Locker::read_lock() {
    return lock(LockAccessType::READ, LockBlockingType::NON_BLOCKING);
}

bool Locker::write_lock() {
    return lock(LockAccessType::WRITE, LockBlockingType::NON_BLOCKING);
}

bool Locker::lock(LockAccessType access, LockBlockingType blocking) {
    int fcntl_flags = 0;
    short type = 0;
    switch (access) {
        case LockAccessType::READ: {
            type = F_RDLCK;
        } break;
        case LockAccessType::WRITE: {
            type = F_WRLCK;
        } break;
    }
    switch (blocking) {
        case LockBlockingType::BLOCKING: {
            fcntl_flags |= F_SETLKW;
        } break;
        case LockBlockingType::NON_BLOCKING: {
            fcntl_flags |= F_SETLK;
        } break;
    }

    if (lock_fd == -1) {
        lock_fd = open(path.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0660);
        if (lock_fd == -1) {
            throw SystemError(errno, M_("Failed to open lock file \"{}\""), path);
        }
    }

    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    auto rc = fcntl(lock_fd, fcntl_flags, &fl);
    if (rc == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            return false;
        } else {
            throw SystemError(errno, M_("Failed to obtain lock \"{}\""), path);
        }
    }

    return true;
}

void Locker::unlock() {
    if (lock_fd != -1) {
        if (close(lock_fd) == -1) {
            throw SystemError(errno, M_("Failed to close lock file \"{}\""), path);
        }
        if (unlink(path.c_str()) == -1) {
            throw SystemError(errno, M_("Failed to delete lock file \"{}\""), path);
        }
        lock_fd = -1;
    }
}

Locker::~Locker() {
    try {
        unlock();
    } catch (...) {
    }
}

}  // namespace libdnf5::utils
