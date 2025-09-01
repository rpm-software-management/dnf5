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

#ifndef LIBDNF5_UTILS_FS_FILE_HPP
#define LIBDNF5_UTILS_FS_FILE_HPP

#include "libdnf5/defs.h"

#include <cstdio>
#include <filesystem>


namespace libdnf5::utils::fs {

/// A wrapper for a `FILE *` that handles opening and closing a file in RAII
/// fashion. Errors are handled by raising instances of
/// `libdnf5::FileSystemError` so that there's a single exception type
/// being raised for all filesystem-related errors.
class LIBDNF_API File {
public:
    /// Creates an instance of `File` without opening any file.
    File();

    /// Creates an instance of `File` and opens the file at `path` using mode `mode`.
    ///
    /// @param path The path of the file.
    /// @param mode The mode for opening the file.
    /// @param use_solv_xfopen Use libsolv's solv_xfopen to transparently work
    ///                        with compressed files (based on filename
    ///                        extension).
    File(const std::filesystem::path & path, const char * mode, bool use_solv_xfopen = false);

    /// Creates an instance of `File` by opening file descriptor `fd`. The
    /// `path` argument is only used for error reporting and for retrieving from
    /// the object via `get_path()`.
    ///
    /// @param fd The file descriptor to call fdopen() on.
    /// @param path The path of the file, for error reporting etc.
    /// @param mode The mode for opening the file.
    /// @param use_solv_xfopen_fd Use libsolv's solv_xfopen_fd to transparently
    ///                           work with compressed files (based on filename
    ///                           extension).
    File(int fd, const std::filesystem::path & path, const char * mode, bool use_solv_xfopen_fd = false);

    File(const File &) = delete;
    File & operator=(const File &) = delete;
    File(File && f) noexcept;
    File & operator=(File && f);
    ~File();

    /// Opens the file at `path` using mode `mode`. If this object already has
    /// an open file, closes it first before opening the new one.
    ///
    /// @param path The path of the file.
    /// @param mode The mode for opening the file.
    /// @param use_solv_xfopen Use libsolv's solv_xfopen to transparently work
    ///                        with compressed files (based on filename
    ///                        extension).
    void open(const std::filesystem::path & path, const char * mode, bool use_solv_xfopen = false);

    /// Opens the file by opening file descriptor `fd` (via `fdopen()`). The
    /// `path` argument is only used for error reporting and for retrieving from
    /// the object via `get_path()`.
    ///
    /// @param fd The file descriptor to call `fdopen()` on.
    /// @param path The path of the file, for error reporting etc.
    /// @param mode The mode for opening the file.
    /// @param use_solv_xfopen_fd Use libsolv's solv_xfopen_fd to transparently
    ///                           work with compressed files (based on filename
    ///                           extension).
    void open(int fd, const std::filesystem::path & path, const char * mode, bool use_solv_xfopen_fd = false);

    /// Close the file.
    void close();

    /// Releases the file, meaning it will no longer be closed on destruction.
    std::FILE * release() noexcept;

    /// Reads at most `count` chars into `buffer`. If EOF is reached, returns a
    /// number smaller than `count`.
    ///
    /// @param buffer The data buffer to read into.
    /// @param count The size of `buffer`.
    /// @return The number of chars read.
    std::size_t read(void * buffer, std::size_t count);

    /// Writes `count` chars from `buffer.
    ///
    /// @param buffer The data buffer to write.
    /// @param count The number of chars to write from `buffer`.
    void write(const void * buffer, std::size_t count);

    /// Reads a single char from the file.
    ///
    /// @param c The char variable to read to.
    /// @return `true` if char was read, `false` in case EOF was reached.
    bool getc(char & c);

    /// Writes a single char to the file.
    ///
    /// @param c The char to write.
    void putc(char c);

    /// Flushes the data from the internal FILE stream buffer.
    void flush();

    /// Seeks to `offset` relative to `whence`. The values for `whence` are
    /// `SEEK_SET`, `SEEK_CUR` and `SEEK_END`, directly from the `<cstdio>`
    /// header.
    ///
    /// @param offset The offset to seek to.
    /// @param whence The relative position to seek from.
    void seek(long offset, int whence);

    /// @return The current position indicator value of the FILE stream.
    long tell() const;

    /// Rewinds the FILE stream to the beginning.
    void rewind();

    /// @return Whether the current position indicator is at the end of file (EOF).
    bool is_at_eof() const;

    /// Reads the contents of the file from current position to the end or until `count` chars are read.
    ///
    /// It will try to detect the number of characters in the file until the end. If the detection is successful,
    /// the required memory is allocated at once. Otherwise, the fallback solution reads the file block by block
    /// and reallocates memory.
    ///
    /// @param count The maximum number of characters to read, 0 to read till the end.
    /// @return The contents read from the file.
    std::string read(std::size_t count = 0);

    /// Reads a single line of the file.
    ///
    /// @param line The string variable to read the line into.
    /// @return `true` if line was read, `false` in case EOF was reached.
    bool read_line(std::string & line);

    /// Writes `data` into the file.
    ///
    /// @param data The data to write.
    void write(std::string_view data);

    /// @return Whether this object contains an open file.
    explicit operator bool() const noexcept;

    /// Returns the associated file path.
    /// @return The associated file path.
    const std::filesystem::path & get_path() const noexcept;

    /// Returns the associated open stream or nullptr.
    /// @return The associated open stream or nullptr.
    std::FILE * get() const noexcept;

    /// Returns the associated open file descriptor.
    /// The operation requires an open file. Throws a `libdnf5::FileSystemError` exception if an error occurs.
    /// @return The associated open file descriptor.
    int get_fd() const;

private:
    std::filesystem::path path;
    std::FILE * file = nullptr;
};

}  // namespace libdnf5::utils::fs

#endif  // LIBDNF5_UTILS_FS_FILE_HPP
