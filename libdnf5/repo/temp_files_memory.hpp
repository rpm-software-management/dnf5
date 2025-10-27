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

#ifndef LIBDNF5_REPO_TEMP_FILES_MEMORY_HPP
#define LIBDNF5_REPO_TEMP_FILES_MEMORY_HPP

#include "libdnf5/base/base_weak.hpp"

#include <filesystem>
#include <vector>


namespace libdnf5::repo {

/// @brief A class for storing paths of temporary files in a TOML file.
/// It behaves in a stateless way, meaning no data are cached within the
/// class.
class TempFilesMemory {
public:
    /// @brief Filename in which the paths are stored.
    static constexpr const char * MEMORY_FILENAME = "temporary_files.toml";

    /// @brief TOML key used for the array of stored paths.
    static constexpr const char * FILE_PATHS_TOML_KEY = "files";

    /// @brief Create the object for managing temporary files' paths.
    /// @param base       A weak pointer to Base object.
    /// @param parent_dir Path to a directory where the memory file is or will be stored.
    ///                   If the directory doesn't exist yet, it will be created.
    /// @exception std::filesystem::filesystem_error When an error occurs during creating the parent directory.
    TempFilesMemory(const BaseWeakPtr & base, const std::string & parent_dir);
    ~TempFilesMemory();

    /// @brief Retrieve stored paths of temporary files.
    /// @return A vector of parsed paths of temporary files.
    /// @exception libdnf5::Error When an error occurs during parsing of the file with temporary files.
    /// @exception std::filesystem::filesystem_error When an error occurs during accessing the memory file.
    std::vector<std::string> get_files() const;

    /// @brief Add a list of file paths into the memory file.
    /// The memory file is created if it didn't exist before.
    /// Existing paths are loaded first using the `get_files()` method.
    /// The resulting list contains both existing and new paths and it's sorted and deduplicated before storing in the file.
    /// @param paths A list of file paths to be added into the memory file.
    /// @exception libdnf5::Error When an error occurs during parsing of the file with temporary files.
    /// @exception libdnf5::FileSystemprror When an error occurs during accessing or writing the memory file.
    /// @exception std::filesystem::filesystem_error When an error occurs during renaming the memory file.
    void add_files(const std::vector<std::string> & paths);

    /// @brief Remove a list of file paths from the memory file.
    /// The memory file is created if it didn't exist before.
    /// The resulting list contains paths that were in the memory file but were not in `paths`.
    /// @param paths A list of file paths to be removed from the memory file.
    /// @exception libdnf5::Error When an error occurs during parsing of the file with temporary files.
    /// @exception libdnf5::FileSystemprror When an error occurs during accessing or writing the memory file.
    /// @exception std::filesystem::filesystem_error When an error occurs during renaming the memory file.
    void remove_files(const std::vector<std::string> & paths);

    /// @brief Deletes the memory file.
    /// @exception std::filesystem::filesystem_error When an error occurs during deleting the memory file.
    void clear();

private:
    BaseWeakPtr base;
    std::filesystem::path full_memory_path;
    void write(const std::vector<std::string> & paths);
};


}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_TEMP_FILES_MEMORY_HPP
