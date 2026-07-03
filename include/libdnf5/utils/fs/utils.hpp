// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_FS_UTILS_HPP
#define LIBDNF5_UTILS_FS_UTILS_HPP

#include "libdnf5/defs.h"

#include <filesystem>
#include <string_view>
#include <system_error>
#include <vector>


namespace libdnf5::utils::fs {

/// Creates an alphabetically sorted, **unique** list of files with the given `file_extension` found
/// in the specified `directories`. If a file with the same name is found in multiple directories,
/// only the one from the directory traversed first is added to the list.
/// Note: Directories are traversed in the same order as they are provided in the input vector.
///
/// @param directories The vector of filesystem paths to search for files.
/// @param file_extension The extension of files to include in the list (e.g., ".repo").
/// @return A vector of alphabetically sorted, unique filesystem paths to regular files or links to regular files.
[[nodiscard]] std::vector<std::filesystem::path> LIBDNF_API
create_sorted_file_list(const std::vector<std::filesystem::path> & directories, std::string_view file_extension);

/// Like std::filesystem::copy(src, dest, copy_options::overwrite_existing), but
/// performs a reflink (copy-on-write clone via the FICLONE ioctl) when the
/// filesystem supports it, falling back to a plain byte copy otherwise. Sets `ec`
/// on error and clears it otherwise. Never throws.
void LIBDNF_API
reflink_or_copy(const std::filesystem::path & src, const std::filesystem::path & dest, std::error_code & ec) noexcept;

}  // namespace libdnf5::utils::fs

#endif  // LIBDNF5_UTILS_FS_UTILS_HPP
