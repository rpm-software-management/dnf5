// Copyright Contributors to the DNF5 project.
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


#ifndef DNF5_COMMANDS_CLEAN_CLEAN_HPP
#define DNF5_COMMANDS_CLEAN_CLEAN_HPP

#include <dnf5/context.hpp>

#include <memory>


namespace dnf5 {


class CleanCommand : public Command {
public:
    explicit CleanCommand(Context & context) : Command(context, "clean") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

    enum Actions : unsigned {
        NONE = 0,
        CLEAN_ALL = 1 << 0,
        CLEAN_PACKAGES = 1 << 1,
        CLEAN_DBCACHE = 1 << 2,
        CLEAN_METADATA = 1 << 3,
        EXPIRE_CACHE = 1 << 4
    };

private:
    Actions required_actions = NONE;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_CLEAN_CLEAN_HPP
