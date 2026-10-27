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

#ifndef LIBDNF5_UTILS_FS_UTILS_PRIVATE_HPP
#define LIBDNF5_UTILS_FS_UTILS_PRIVATE_HPP

#include "libdnf5/utils/fs/utils.hpp"


namespace libdnf5::utils::fs {

/// Compares content of two files.
/// Returns "true" if files have the same content.
/// If content differs or error occurred (file doesn't exist, not readable, ...) returns "false".
[[nodiscard]] bool have_files_same_content_noexcept(const char * file_path1, const char * file_path2) noexcept;

/// Recursive renames/moves file/directory.
/// Implements copy and remove fallback.
void move_recursive(const std::filesystem::path & src, const std::filesystem::path & dest);

/// Adds read permission for group and others to a file or directory (directories also get search permission),
/// regardless of the process umask. Used for state and configuration that must stay readable by unprivileged users.
/// Nothing is changed when the permissions are already there, so it also works for files the caller does not own.
void add_read_for_all(const std::filesystem::path & path, std::error_code & ec);

/// Like `std::filesystem::create_directories()`, but the directories it creates (including missing parents)
/// are also made readable and searchable by group and others regardless of the process umask.
/// Directories that already exist keep their permissions.
void create_directories_readable(const std::filesystem::path & path, std::error_code & ec);

}  // namespace libdnf5::utils::fs

#endif
