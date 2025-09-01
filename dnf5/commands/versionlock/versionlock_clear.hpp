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

#ifndef DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_CLEAR_HPP
#define DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_CLEAR_HPP


#include <dnf5/context.hpp>


namespace dnf5 {


class VersionlockClearCommand : public Command {
public:
    explicit VersionlockClearCommand(Context & context) : VersionlockClearCommand(context, "clear") {}
    void set_argument_parser() override;
    void run() override;

protected:
    VersionlockClearCommand(Context & context, const std::string & name) : Command(context, name) {}
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_CLEAR_HPP
