// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_HPP
#define DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class VersionlockCommand : public Command {
public:
    explicit VersionlockCommand(Context & context) : Command(context, "versionlock") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_HPP
