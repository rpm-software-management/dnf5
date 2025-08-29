// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_MODULE_MODULE_REMOVE_HPP
#define DNF5_COMMANDS_MODULE_MODULE_REMOVE_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class ModuleRemoveCommand : public Command {
public:
    explicit ModuleRemoveCommand(Context & context) : Command(context, "remove") {}
    void set_argument_parser() override;
    void run() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MODULE_MODULE_REMOVE_HPP
