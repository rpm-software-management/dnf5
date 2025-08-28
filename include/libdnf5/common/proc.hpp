// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_COMMON_PROC_HPP
#define LIBDNF5_COMMON_PROC_HPP

#include "libdnf5/defs.h"

#include <sys/types.h>


namespace libdnf5 {

constexpr uid_t INVALID_UID = static_cast<uid_t>(-1);

/// Read the process owner login uid from the "/proc/<pid>/loginuid".
/// @param pid process id
/// @return libdnf5::INVALID_UID if fails, login uid otherwise
/// @since 5.0
LIBDNF_API uid_t read_login_uid_from_proc(pid_t pid) noexcept;

/// Return the current user login uid, if available.
/// The getuid() is returned instead if there was a problem.
/// The value is cached.
/// @since 5.0
LIBDNF_API uid_t get_login_uid() noexcept;

}  // namespace libdnf5

#endif
