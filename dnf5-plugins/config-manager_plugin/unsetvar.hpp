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


#ifndef DNF5_COMMANDS_CONFIG_MANAGER_UNSETVAR_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_UNSETVAR_HPP

#include <dnf5/context.hpp>

#include <set>
#include <string>

namespace dnf5 {

class ConfigManagerUnsetVarCommand : public Command {
public:
    explicit ConfigManagerUnsetVarCommand(Context & context) : Command(context, "unsetvar") {}
    void set_argument_parser() override;
    void configure() override;

private:
    std::set<std::string> vars_to_remove;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_UNSETVAR_HPP
