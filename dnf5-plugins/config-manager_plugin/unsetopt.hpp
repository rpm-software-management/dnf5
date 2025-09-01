// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef DNF5_COMMANDS_CONFIG_MANAGER_UNSETOPT_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_UNSETOPT_HPP

#include <dnf5/context.hpp>

#include <map>
#include <set>
#include <string>

namespace dnf5 {

class ConfigManagerUnsetOptCommand : public Command {
public:
    explicit ConfigManagerUnsetOptCommand(Context & context) : Command(context, "unsetopt") {}
    void set_argument_parser() override;
    void configure() override;

private:
    libdnf5::ConfigMain tmp_config;
    libdnf5::repo::ConfigRepo tmp_repo_conf{tmp_config, "temporary_to_check_repository_options"};
    std::set<std::string> main_opts_to_remove;
    std::map<std::string, std::set<std::string>> in_repos_opts_to_remove;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_UNSETOPT_HPP
