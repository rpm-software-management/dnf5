/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_UTILS_TEMP_HPP
#define LIBDNF_UTILS_TEMP_HPP

#include <filesystem>
#include <string>


namespace libdnf::utils {

/// Object that creates and holds a temp directory.
/// The directory gets removed when the object is deleted.
class TempDir {
public:
    /// Creates a temporary directory in the system temporary directory path.
    explicit TempDir(const std::string & name_prefix);

    /// Creates a temporary directory in `destdir`.
    TempDir(std::filesystem::path destdir, const std::string & name_prefix);

    TempDir(const TempDir &) = delete;
    TempDir & operator=(const TempDir &) = delete;

    ~TempDir();
    const std::filesystem::path & get_path() const noexcept { return path; }

private:
    std::filesystem::path path;
};

/// A mkstemp wrapper that creates and owns a temporary file, which will be
/// deleted in the destructor unless released. Throws instances of
/// `std::filesystem::filesystem_error` on any I/O failure.
class TempFile {
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

    TempFile(const TempDir &) = delete;
    TempFile & operator=(const TempFile &) = delete;

    ~TempFile();

    /// Call `::fdopen()` on the file descriptor and open it as a FILE * stream.
    ///
    /// @param mode The mode for the file, passed to `::fdopen()`.
    FILE * fdopen(const char * mode);

    /// Close either the FILE * (if fdopened before) or the file descriptor.
    void close();

    /// Releases the temporary file, meaning it will no longer be closed or
    /// deleted on destruction.
    void release() noexcept;

    const std::filesystem::path & get_path() const noexcept { return path; }
    int get_fd() const noexcept { return fd; }
    FILE * get_file() const noexcept { return file; }

private:
    std::filesystem::path path;
    int fd = -1;
    FILE * file = nullptr;
};

}  // namespace libdnf::utils

#endif  // LIBDNF_UTILS_TEMP_HPP
