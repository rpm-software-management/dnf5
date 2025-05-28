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


#ifndef DNF5_COMMANDS_FROM_REPO_HPP
#define DNF5_COMMANDS_FROM_REPO_HPP

#include <dnf5/context.hpp>

#include <string>
#include <vector>


namespace dnf5 {

void create_from_repo_option(Command & command, std::vector<std::string> & from_repos, bool detect_conflict);

}

#endif  // DNF5_COMMANDS_FROM_REPO_HPP
