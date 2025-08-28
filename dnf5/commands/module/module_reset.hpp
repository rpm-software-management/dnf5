// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_MODULE_MODULE_RESET_HPP
#define DNF5_COMMANDS_MODULE_MODULE_RESET_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class ModuleResetCommand : public Command {
public:
    explicit ModuleResetCommand(Context & context) : Command(context, "reset") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::vector<std::string> module_specs;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MODULE_MODULE_RESET_HPP
