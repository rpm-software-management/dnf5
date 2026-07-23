// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_PLUGIN_SYSTEMD_INHIBITOR_HPP
#define LIBDNF5_PLUGIN_SYSTEMD_INHIBITOR_HPP

#include "libdnf5/base/transaction.hpp"
#include "libdnf5/defs.h"

namespace libdnf5::plugin {

/// Registers a systemd inhibitor lock file descriptor obtained by a plugin's
/// `pre_transaction()` hook. The descriptor is closed automatically when the
/// given transaction run finishes (successfully or not), regardless of which
/// plugins are installed or enabled - so the lock is guaranteed to be released
/// even if the plugin that acquired it does not implement (or is not enabled
/// for) `post_transaction()`.
///
/// Ownership of the file descriptor is transferred; the caller must not close
/// it itself.
///
/// @param transaction The transaction this lock belongs to - the same object
///        passed into the plugin's `pre_transaction()`/`post_transaction()` hooks.
/// @param fd An open inhibitor lock file descriptor. Negative values are ignored.
LIBDNF_API void register_systemd_inhibitor_fd(const libdnf5::base::Transaction & transaction, int fd);

}  // namespace libdnf5::plugin

#endif
