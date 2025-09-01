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

#ifndef LIBDNF5_UTILS_FS_TEMP_HPP
#define LIBDNF5_UTILS_FS_TEMP_HPP

#include "file.hpp"

#include "libdnf5/defs.h"

#include <filesystem>
#include <optional>
#include <string>


namespace libdnf5::utils::fs {

/// Object that creates and holds a temp directory.
/// The directory gets removed when the object is deleted.
class LIBDNF_API TempDir {
public:
    /// Creates a temporary directory in the system temporary directory path.
    explicit TempDir(const std::string & name_prefix);

    /// Creates a temporary directory in `destdir`.
    TempDir(std::filesystem::path destdir, const std::string & name_prefix);

    TempDir(const TempDir &) = delete;
    TempDir(TempDir && src) noexcept;

    TempDir & operator=(const TempDir &) = delete;
    TempDir & operator=(TempDir && src) noexcept;

    ~TempDir();

    /// Releases the temporary directory, meaning it will no longer be deleted
    /// on destruction.
    void release() noexcept;

    const std::filesystem::path & get_path() const noexcept { return path; }

private:
    std::filesystem::path path;
};

/// A mkstemp wrapper that creates and owns a temporary file, which will be
/// deleted in the destructor unless released. Throws instances of
/// `libdnf5::FileSystemError` on any I/O failure.
class LIBDNF_API TempFile {
public:
    /// Creates a temporary file in the system temporary directory path.
    ///
    /// @param name_prefix The prefix of the filename to which ".XXXXXX" will be appended.
    explicit TempFile(const std::string & name_prefix);

    /// Creates a temporary file in `destdir`.
    ///
    /// @param destdir The directory in which the file will be created.
    /// @param name_prefix The prefix of the filename to which ".XXXXXX" will be appended.
    TempFile(std::filesystem::path destdir, const std::string & name_prefix);

    TempFile(const TempFile &) = delete;
    TempFile(TempFile && src) noexcept;

    TempFile & operator=(const TempFile &) = delete;
    TempFile & operator=(TempFile &&);

    ~TempFile();

    /// Open the TempFile as a File object.
    ///
    /// @param mode The mode for the file, passed to `::fdopen()`.
    File & open_as_file(const char * mode);

    /// If this TempFile has been opened as File (via `open_as_file()`), unsets
    /// and destroys that File (automatically closing upon destruction).
    /// Otherwise closes the open file descriptor.
    void close();

    /// Releases the temporary file, meaning it will no longer be closed or
    /// deleted on destruction. If open as File (via `open_as_file()`), releases
    /// the File by calling its `release()` method and unsets and destroys the
    /// File.
    void release() noexcept;

    const std::filesystem::path & get_path() const noexcept { return path; }
    int get_fd() const noexcept { return fd; }
    std::optional<File> & get_file() noexcept { return file; }

private:
    std::filesystem::path path;
    int fd = -1;
    std::optional<File> file;
};

}  // namespace libdnf5::utils::fs

#endif  // LIBDNF5_UTILS_FS_TEMP_HPP
