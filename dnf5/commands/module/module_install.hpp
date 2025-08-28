// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_MODULE_MODULE_INSTALL_HPP
#define DNF5_COMMANDS_MODULE_MODULE_INSTALL_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class ModuleInstallCommand : public Command {
public:
    explicit ModuleInstallCommand(Context & context) : Command(context, "install") {}
    void set_argument_parser() override;
    void run() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MODULE_MODULE_INSTALL_HPP
