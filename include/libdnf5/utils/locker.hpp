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

#ifndef LIBDNF5_UTILS_LOCKER_HPP
#define LIBDNF5_UTILS_LOCKER_HPP

#include "libdnf5/defs.h"

#include <string>

namespace libdnf5::utils {

enum class LIBDNF_API LockAccessType { READ, WRITE };
enum class LIBDNF_API LockBlockingType { NON_BLOCKING, BLOCKING };

/// Object for implementing a simple file mutex mechanism
/// or checking read/write access on a given path.
class LIBDNF_API Locker {
public:
    /// Create a Locker object at a given path
    explicit Locker(const std::string & path);
    ~Locker();

    /// @brief Acquire a read or write lock on a given file path, either blocking or non-blocking.
    /// @return True if lock acquisition was successful, otherwise false
    /// @throws libdnf5::SystemError if an unexpected error occurs when checking the lock state, like insufficient privileges
    bool lock(LockAccessType access, LockBlockingType blocking);

    /// @brief Try to acquire read lock on a given file path
    /// @return True if lock acquisition was successful, otherwise false
    /// @throws libdnf5::SystemError if an unexpected error occurs when checking the lock state, like insufficient privileges
    bool read_lock();

    /// @brief Try to acquire write lock on a given file path
    /// @return True if lock acquisition was successful, otherwise false
    /// @throws libdnf5::SystemError if an unexpected error occurs when checking the lock state, like insufficient privileges
    bool write_lock();

    /// @brief Unlock the existing lock and remove the underlying lock file
    /// @throws libdnf5::SystemError if an unexpected error occurs when unlocking
    void unlock();

private:
    std::string path;
    int lock_fd{-1};
};

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_LOCKER_HPP
