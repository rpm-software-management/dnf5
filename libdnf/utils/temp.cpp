/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "temp.hpp"

#include <cstdlib>


namespace libdnf::utils {


TempDir::TempDir(const std::string & prefix) {
    // get path to the system temp directory
    auto temp = std::filesystem::temp_directory_path();

    // append /<prefix>XXXXXX as requested in mkdtemp(3)
    temp /= prefix + "XXXXXX";
    path = mkdtemp(const_cast<char *>(temp.native().c_str()));
}


TempDir::TempDir(const std::string & prefix, const std::vector<std::string> & subdirs)
    : TempDir(prefix) {
    for (auto & subdir : subdirs) {
        std::filesystem::create_directory(path / subdir);
    }
}


TempDir::~TempDir() {
    // remove the temp directory and all its content
    try {
        std::filesystem::remove_all(path);
    } catch (std::filesystem::filesystem_error &) {
        // catch an exception that shouldn't be raised in a destructor
        // we should consider logging or printing the exception
    }
}


}  // namespace libdnf::utils
