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

#ifndef DNF5_COMMANDS_COPR_HELPERS_HPP
#define DNF5_COMMANDS_COPR_HELPERS_HPP

#include <libdnf5/utils/format.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace dnf5 {

std::vector<std::string> repo_fallbacks(const std::string & name_version);
template <typename... Args>
void warning(const char * format, Args &&... args) {
    std::cerr << "WARNING: " + libdnf5::utils::sformat(format, std::forward<Args>(args)...) << std::endl;
}

}  // namespace dnf5

#endif  // DNF5_COMMANDS_COPR_HELPERS_HPP
