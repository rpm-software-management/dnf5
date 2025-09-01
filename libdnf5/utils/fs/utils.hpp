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

#ifndef LIBDNF5_UTILS_FS_UTILS_HPP
#define LIBDNF5_UTILS_FS_UTILS_HPP

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>


namespace libdnf5::utils::fs {

/// Compares content of two files.
/// Returns "true" if files have the same content.
/// If content differs or error occurred (file doesn't exist, not readable, ...) returns "false".
[[nodiscard]] bool have_files_same_content_noexcept(const char * file_path1, const char * file_path2) noexcept;

/// Recursive renames/moves file/directory.
/// Implements copy and remove fallback.
void move_recursive(const std::filesystem::path & src, const std::filesystem::path & dest);

// Creates an alphabetically sorted list of all files with `file_extension` from `directories`.
// If a file with the same name is in multiple directories, only the first file found is added to the list.
// Directories are traversed in the same order as they are in the input vector.
[[nodiscard]] std::vector<std::filesystem::path> create_sorted_file_list(
    const std::vector<std::filesystem::path> & directories, std::string_view file_extension);

}  // namespace libdnf5::utils::fs

#endif  // LIBDNF5_UTILS_FS_UTILS_HPP
