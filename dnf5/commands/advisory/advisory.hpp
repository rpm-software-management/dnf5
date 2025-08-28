// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class AdvisoryCommand : public Command {
public:
    explicit AdvisoryCommand(Context & context) : Command(context, "advisory") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_HPP
