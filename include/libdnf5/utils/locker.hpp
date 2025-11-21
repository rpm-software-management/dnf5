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

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <filesystem>

namespace libdnf5::utils {

enum class LIBDNF_API LockAccess { READ, WRITE };
enum class LIBDNF_API LockBlocking { NON_BLOCKING, BLOCKING };

/// Object for implementing a simple file mutex mechanism
/// or checking read/write access on a given path.
class LIBDNF_API Locker {
public:
    /// Create a Locker object at a given path
    Locker(const std::filesystem::path & path, bool keep_file = false);
    ~Locker();

    Locker(const Locker &) = delete;
    Locker & operator=(const Locker &) = delete;

    Locker(Locker &&) noexcept;
    Locker & operator=(Locker &&) noexcept;

    /// @brief Acquire a read or write lock on a given file path, either blocking or non-blocking.
    /// @return True if lock acquisition was successful, otherwise false
    /// @throws libdnf5::SystemError if an unexpected error occurs when checking the lock state, like insufficient privileges
    bool lock(LockAccess type, LockBlocking blocking);

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

    /// @brief Open the file but don't acquire a lock yet. Useful for checking permissions.
    /// @throws libdnf5::SystemError if an unexpected error occurs when opening the file
    void open_file(LockAccess type);

    /// @brief Get the path being locked
    const std::filesystem::path & get_path() const noexcept;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_LOCKER_HPP
