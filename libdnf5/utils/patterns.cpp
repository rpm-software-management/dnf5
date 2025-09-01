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

#include "libdnf5/utils/patterns.hpp"

#include <cstring>

namespace libdnf5::utils {

bool is_glob_pattern(const char * pattern) noexcept {
    return strpbrk(pattern, "*[?") != nullptr;
}

bool is_file_pattern(const std::string & pattern) noexcept {
    return pattern[0] == '/' || (pattern[0] == '*' && pattern[1] == '/');
}

}  // namespace libdnf5::utils
