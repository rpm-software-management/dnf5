// Copyright Contributors to the DNF5 project.
// Copyright (C) 2025 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_BOOTC_HPP
#define LIBDNF5_BOOTC_HPP

#include "libdnf5/defs.h"

namespace libdnf5::utils::bootc {

/// @brief Call `bootc status` to check whether the system is deployed via
/// bootc
///
/// Per bootc documentation, "bootc is not the only image based system; there
/// are many. Detect bootc specifically via `bootc status`." Works on both
/// OSTree and composefs-backed systems.
///
/// @throws libdnf5::RuntimeError if the bootc command is available but cannot
/// be called, or it exits with non-zero code, or it outputs invalid JSON
/// @returns true if the bootc command is available and `bootc status --json`
/// has a .spec.image. Otherwise, returns false.
LIBDNF_API bool is_bootc_system();

/// @brief Check whether /usr is mounted read/write or read-only
/// @throws libdnf5::SystemError if fails to stat /usr
/// @returns false if the filesystem backing /usr is mounted read-only, true
/// otherwise
LIBDNF_API bool is_writable();

/// @brief Check whether the bootc system has a read-only overlay on /usr
///
/// Calls `bootc status --json` and checks whether
/// .status.usrOverlay.accessMode is "readOnly".
///
/// @throws libdnf5::SystemError if bootc command could not be executed
/// @throws libdnf5::RuntimeError if bootc command exits with non-zero code
LIBDNF_API bool has_read_only_usr_overlay();

/// @brief Make /usr writable on bootc systems using a read-only overlay.
///
/// If /usr is already writable, this is a no-op. Otherwise, creates a
/// read-only overlay over /usr (`bootc usr-overlay --read-only`) if none
/// exists, and remounts /usr as read/write in a private mount namespace so
/// this process can write to it.
///
/// @throws libdnf5::SystemError if unshare() fails, or if bootc command
/// could not be executed
/// @throws libdnf5::RuntimeError if the mount remount command fails, or if
/// `bootc usr-overlay --read-only` fails
LIBDNF_API void make_usr_writable();

}  // namespace libdnf5::utils::bootc

#endif  // LIBDNF5_BOOTC_HPP
