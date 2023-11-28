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


#include "libdnf5/utils/fs/temp.hpp"

#include "libdnf5/common/exception.hpp"

#include <stdio.h>
#include <unistd.h>

#include <cstdlib>


namespace libdnf5::utils::fs {


TempDir::TempDir(const std::string & name_prefix) : TempDir(std::filesystem::temp_directory_path(), name_prefix) {}


TempDir::TempDir(std::filesystem::path destdir, const std::string & name_prefix) {
    destdir /= name_prefix + ".XXXXXX";

    // mkdtemp modifies the dest's char * in-place
    std::string dest = destdir;
    const char * temp_path = mkdtemp(dest.data());
    if (temp_path == nullptr) {
        throw std::filesystem::filesystem_error(
            "cannot create temporary directory", destdir, std::error_code(errno, std::system_category()));
    }
    path = temp_path;
}


TempDir::TempDir(TempDir && src) noexcept : path(std::move(src.path)) {
    src.path.clear();
}


TempDir & TempDir::operator=(TempDir && src) noexcept {
    if (&src != this) {
        path = std::move(src.path);
        src.path.clear();
    }
    return *this;
}


TempDir::~TempDir() {
    try {
        if (!path.empty()) {
            std::filesystem::remove_all(path);
        }
    } catch (std::exception &) {
        // catch an exception that shouldn't be raised in a destructor
        // TODO(lukash) consider logging or printing the exception
    }
}


void TempDir::release() noexcept {
    path.clear();
}


TempFile::TempFile(const std::string & name_prefix) : TempFile(std::filesystem::temp_directory_path(), name_prefix) {}


TempFile::TempFile(std::filesystem::path destdir, const std::string & name_prefix) {
    destdir /= name_prefix + ".XXXXXX";

    // mkstemp modifies the dest's char * in-place
    std::string dest = destdir;
    fd = mkstemp(dest.data());
    if (fd == -1) {
        throw std::filesystem::filesystem_error(
            "cannot create temporary file", destdir, std::error_code(errno, std::system_category()));
    }

    path = dest;
}


TempFile::TempFile(TempFile && src) noexcept : path(std::move(src.path)), fd(src.fd), file(std::move(src.file)) {
    src.path.clear();
    src.fd = -1;
    src.file.reset();
}


TempFile & TempFile::operator=(TempFile && src) {
    if (&src != this) {
        file = std::move(src.file);
        path = std::move(src.path);
        src.path.clear();
        fd = src.fd;
        src.fd = -1;
    }
    return *this;
}


TempFile::~TempFile() {
    try {
        close();
    } catch (std::exception &) {
        // TODO(lukash) consider logging or printing the exception
    }

    if (!path.empty()) {
        unlink(path.c_str());
    }
}


File & TempFile::open_as_file(const char * mode) {
    libdnf_assert(fd != -1, "Cannot open as file TempFile that has been closed or released");
    libdnf_assert(!file, "This TempFile has already been opened as File");

    file.emplace(fd, path, mode);
    return *file;
}


void TempFile::close() {
    if (file) {
        file.reset();
    } else if (fd != -1) {
        if (::close(fd) != 0) {
            throw std::filesystem::filesystem_error(
                "cannot close temporary file", path, std::error_code(errno, std::system_category()));
        }
    }

    fd = -1;
}


void TempFile::release() noexcept {
    if (file) {
        file->release();
        file.reset();
    }
    fd = -1;
    path.clear();
}

}  // namespace libdnf5::utils::fs
