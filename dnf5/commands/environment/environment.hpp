// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_HPP
#define DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class EnvironmentCommand : public Command {
public:
    explicit EnvironmentCommand(Context & context) : Command(context, "environment") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_HPP
