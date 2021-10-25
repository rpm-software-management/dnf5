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


#include "locker.hpp"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace libdnf::utils {

Locker::LockResult Locker::lock() {
    lock_fd = open(path.c_str(), O_CREAT | O_RDWR, 0660);
    if (lock_fd == -1) {
        return LockResult::ERROR_FD;
    }

    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    auto rc = fcntl(lock_fd, F_SETLK, &fl);
    if (rc == -1) {
        return LockResult::ERROR_LOCK;
    }

    return LockResult::SUCCESS;
}

void Locker::unlock() {
    if (lock_fd != -1) {
        unlink(path.c_str());
        close(lock_fd);
    }
}

Locker::~Locker() {
    unlock();
}

}  // namespace libdnf::utils
