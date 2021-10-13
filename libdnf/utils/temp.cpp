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

#include "libdnf/common/exception.hpp"
#include "libdnf/utils/bgettext/bgettext-lib.h"

#include <fmt/format.h>

#include <cstdlib>


namespace libdnf::utils {


TempDir::TempDir(const std::string & prefix) : TempDir(std::filesystem::temp_directory_path(), prefix) {}


TempDir::TempDir(const std::string & destdir, const std::string & prefix) {
    std::filesystem::path dir = destdir;
    dir /= prefix + "XXXXXX";

    const char * temp_path = mkdtemp(const_cast<char *>(dir.native().c_str()));
    if (temp_path == nullptr) {
        throw std::filesystem::filesystem_error(
            "cannot create temporary directory",
            dir,
            std::error_code(errno, std::system_category()));
    }
    path = temp_path;
}


TempDir::~TempDir() {
    try {
        std::filesystem::remove_all(path);
    } catch (std::filesystem::filesystem_error &) {
        // catch an exception that shouldn't be raised in a destructor
        // TODO(lukash) consider logging or printing the exception
    }
}


}  // namespace libdnf::utils
