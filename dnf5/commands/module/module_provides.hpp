// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_MODULE_MODULE_PROVIDES_HPP
#define DNF5_COMMANDS_MODULE_MODULE_PROVIDES_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class ModuleProvidesCommand : public Command {
public:
    explicit ModuleProvidesCommand(Context & context) : Command(context, "provides") {}
    void set_argument_parser() override;
    void run() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MODULE_MODULE_PROVIDES_HPP
