/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef DNF5_COMMANDS_ALIASES_REPOLIST_HPP
#define DNF5_COMMANDS_ALIASES_REPOLIST_HPP


#include "commands/repo/repo_list.hpp"


namespace dnf5 {


class RepolistAlias : public RepoListCommand {
public:
    explicit RepolistAlias(Command & parent) : RepoListCommand(parent, "repolist") {
        auto & cmd = *get_argument_parser_command();
        cmd.set_description("Alias for 'repo list'");
    }
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ALIASES_REPOLIST_HPP
