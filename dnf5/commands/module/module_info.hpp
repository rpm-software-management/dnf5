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

#ifndef DNF5_COMMANDS_MODULE_MODULE_INFO_HPP
#define DNF5_COMMANDS_MODULE_MODULE_INFO_HPP

#include "module_list.hpp"

namespace dnf5 {

class ModuleInfoCommand : public ModuleListCommand {
public:
    explicit ModuleInfoCommand(Context & context) : ModuleListCommand(context, "info") {}

private:
    void print(const libdnf5::module::ModuleQuery & query) override;
    void print_hint() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MODULE_MODULE_INFO_HPP
