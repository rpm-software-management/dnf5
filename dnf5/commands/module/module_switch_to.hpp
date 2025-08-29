// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_MODULE_MODULE_SWITCH_TO_HPP
#define DNF5_COMMANDS_MODULE_MODULE_SWITCH_TO_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class ModuleSwitchToCommand : public Command {
public:
    explicit ModuleSwitchToCommand(Context & context) : Command(context, "switch-to") {}
    void set_argument_parser() override;
    void run() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MODULE_MODULE_SWITCH_TO_HPP
