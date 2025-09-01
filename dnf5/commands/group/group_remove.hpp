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

#ifndef DNF5_COMMANDS_GROUP_GROUP_REMOVE_HPP
#define DNF5_COMMANDS_GROUP_GROUP_REMOVE_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class GroupRemoveCommand : public Command {
public:
    explicit GroupRemoveCommand(Context & context) : GroupRemoveCommand(context, "remove") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::unique_ptr<GroupNoPackagesOption> no_packages{nullptr};
    std::unique_ptr<GroupSpecArguments> group_specs{nullptr};

protected:
    // to be used by an alias command only
    explicit GroupRemoveCommand(Context & context, const std::string & name) : Command(context, name) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_GROUP_REMOVE_HPP
