// Copyright Contributors to the DNF5 project.
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

#ifndef DNF5_COMMANDS_STATUS_STATUS_HPP
#define DNF5_COMMANDS_STATUS_STATUS_HPP

#include <dnf5/context.hpp>

namespace dnf5 {


class StatusCommand : public Command {
public:
    explicit StatusCommand(Context & context) : Command(context, "status") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_STATUS_STATUS_HPP
