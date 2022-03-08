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


#include "temp.hpp"

#include "utils/bgettext/bgettext-lib.h"

#include "libdnf/common/exception.hpp"

#include <stdio.h>
#include <unistd.h>

#include <cstdlib>


namespace libdnf::utils::fs {


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


TempDir::~TempDir() {
    try {
        std::filesystem::remove_all(path);
    } catch (std::exception &) {
        // catch an exception that shouldn't be raised in a destructor
        // TODO(lukash) consider logging or printing the exception
    }
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
    path = "";
}

}  // namespace libdnf::utils::fs
