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

#include "config_utils.hpp"

#include "libdnf5/utils/fs/file.hpp"

#include <glob.h>


namespace libdnf5 {


static void add_from_file(std::ostream & out, const std::string & file_path) {
    utils::fs::File file(file_path, "r");

    std::string line;
    while (file.read_line(line)) {
        auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) {
            continue;
        }
        if (line[start] == '#') {
            continue;
        }
        auto end = line.find_last_not_of(" \t\r");

        out.write(line.c_str() + start, static_cast<int>(end - start + 1));
        out.put(' ');
    }
}


static void add_from_files(
    std::ostream & out, const std::string & glob_path, const std::filesystem::path & installroot) {
    // Extend path by installroot
    const auto full_path = (installroot / std::filesystem::path(glob_path).relative_path()).string();
    glob_t glob_buf;
    glob(full_path.c_str(), GLOB_MARK | GLOB_NOSORT, nullptr, &glob_buf);
    for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
        auto path = glob_buf.gl_pathv[i];
        if (path[strlen(path) - 1] != '/') {
            add_from_file(out, path);
        }
    }
    globfree(&glob_buf);
}


std::string resolve_path_globs(const std::string & str_with_globs, const std::filesystem::path & installroot) {
    std::ostringstream res;
    std::string::size_type start{0};
    while (start < str_with_globs.length()) {
        auto end = str_with_globs.find_first_of(" ,\n", start);
        if (str_with_globs.compare(start, 5, "glob:") == 0) {
            start += 5;
            if (start >= str_with_globs.length()) {
                break;
            }
            if (end == std::string::npos) {
                add_from_files(res, str_with_globs.substr(start), installroot);
                break;
            }
            if ((end - start) != 0) {
                add_from_files(res, str_with_globs.substr(start, end - start), installroot);
            }
        } else {
            if (end == std::string::npos) {
                res << str_with_globs.substr(start);
                break;
            }
            if ((end - start) != 0) {
                res << str_with_globs.substr(start, end - start) << " ";
            }
        }
        start = end + 1;
    }
    return res.str();
}


}  // namespace libdnf5
