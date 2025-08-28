// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_LOCKER_HPP
#define LIBDNF5_UTILS_LOCKER_HPP

#include "libdnf5/defs.h"

#include <string>

namespace libdnf5::utils {

/// Object for implementing a simple file mutex mechanism
/// or checking read/write access on a given path.
class LIBDNF_API Locker {
public:
    /// Create a Locker object at a given path
    explicit Locker(const std::string & path);
    ~Locker();

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
    LIBDNF_LOCAL bool lock(short int type);

    std::string path;
    int lock_fd{-1};
};

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_LOCKER_HPP
