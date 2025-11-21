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

constexpr const int MODE = 0664;

namespace libdnf5::utils {

class Locker::Impl {
public:
    Impl(const std::filesystem::path & path, const bool keep_file) : path{path}, keep_file{keep_file} {};
    ~Impl();
    void unlock();

private:
    friend Locker;
    const std::filesystem::path path;
    int lock_fd{-1};
    bool keep_file{false};
};

Locker::Locker(const std::filesystem::path & path, const bool keep_file) : p_impl{new Impl{path, keep_file}} {};

const std::filesystem::path & Locker::get_path() const noexcept {
    return p_impl->path;
}

bool Locker::read_lock() {
    return lock(LockAccess::READ, LockBlocking::NON_BLOCKING);
}

bool Locker::write_lock() {
    return lock(LockAccess::WRITE, LockBlocking::NON_BLOCKING);
}

bool Locker::lock(LockAccess access, LockBlocking blocking) {
    int fcntl_flags = 0;
    short type = 0;
    switch (access) {
        case LockAccess::READ: {
            type = F_RDLCK;
        } break;
        case LockAccess::WRITE: {
            type = F_WRLCK;
        } break;
    }
    switch (blocking) {
        case LockBlocking::BLOCKING: {
            fcntl_flags |= F_SETLKW;
        } break;
        case LockBlocking::NON_BLOCKING: {
            fcntl_flags |= F_SETLK;
        } break;
    }

    if (p_impl->lock_fd == -1) {
        open_file(access);
    }

    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    auto rc = fcntl(p_impl->lock_fd, fcntl_flags, &fl);
    if (rc == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            return false;
        } else {
            throw SystemError(errno, M_("Failed to obtain lock \"{}\""), p_impl->path.string());
        }
    }

    return true;
}

void Locker::open_file(LockAccess access) {
    if (p_impl->lock_fd != -1) {
        throw libdnf5::RuntimeError(M_("File is already open."));
    }
    int open_flags = O_CREAT | O_CLOEXEC;
    switch (access) {
        case LockAccess::READ: {
            open_flags |= O_RDONLY;
        } break;
        case LockAccess::WRITE: {
            open_flags |= O_RDWR;
        } break;
    }
    p_impl->lock_fd = open(p_impl->path.c_str(), open_flags, MODE);
    if (p_impl->lock_fd == -1) {
        throw SystemError(errno, M_("Failed to open lock file \"{}\""), p_impl->path.string());
    }
}

void Locker::Impl::unlock() {
    if (lock_fd != -1) {
        if (close(lock_fd) == -1) {
            throw SystemError(errno, M_("Failed to close lock file \"{}\""), path.string());
        }
        if (!keep_file) {
            if (unlink(path.c_str()) == -1) {
                throw SystemError(errno, M_("Failed to delete lock file \"{}\""), path.string());
            }
        }
        lock_fd = -1;
    }
}

void Locker::unlock() {
    return p_impl->unlock();
}

Locker::Impl::~Impl() {
    try {
        unlock();
    } catch (...) {
    }
}

Locker::~Locker() = default;

Locker::Locker(Locker &&) noexcept = default;

Locker & Locker::operator=(Locker &&) noexcept = default;

}  // namespace libdnf5::utils
