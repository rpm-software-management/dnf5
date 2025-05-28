/*
Copyright Contributors to the dnf5 project.

This file is part of dnf5: https://github.com/rpm-software-management/dnf5/

Dnf5 is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Dnf5 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with dnf5.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef DNF5_COMMANDS_FROM_REPO_HPP
#define DNF5_COMMANDS_FROM_REPO_HPP

#include <dnf5/context.hpp>

#include <string>
#include <vector>


namespace dnf5 {

// Creates a new named argument "--from-repo=REPO_ID,...".
// When the argument is used, the list of REPO_IDs is split and the items are stored in the `from_repos` vector.
// If `detect_conflict` is true, a `libdnf5::cli::ArgumentParserConflictingArgumentsError` exception is thrown
// if "--from-repo" was already defined with a different value.
void create_from_repo_option(Command & command, std::vector<std::string> & from_repos, bool detect_conflict);

}  // namespace dnf5

#endif  // DNF5_COMMANDS_FROM_REPO_HPP
