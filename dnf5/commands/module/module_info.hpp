// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

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
