// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _LIBDNF5_UTILS_SYSTEMD_INHIBIT_HPP
#define _LIBDNF5_UTILS_SYSTEMD_INHIBIT_HPP

#include "libdnf5/logger/logger.hpp"

namespace libdnf5::utils {

/// Acquires a systemd logind inhibitor lock ("shutdown", mode "block") that
/// prevents the system from shutting down or rebooting.
///
/// libsystemd is loaded on demand via dlopen, so libdnf5 has no link-time or
/// runtime dependency on it. If libsystemd is not present, or the lock cannot
/// be acquired for any other reason, the failure is logged and -1 is returned
/// — the caller is expected to treat this as non-fatal.
///
/// @return The lock file descriptor (release it with close()), or -1 on failure.
int acquire_systemd_inhibitor_lock(Logger & logger);

}  // namespace libdnf5::utils

#endif
