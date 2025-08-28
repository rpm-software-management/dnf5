// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_HPP

#include "commands/command.hpp"
#include "context.hpp"

namespace dnfdaemon::client {

class AdvisoryCommand : public DaemonCommand {
public:
    explicit AdvisoryCommand(Context & context) : DaemonCommand(context, "advisory") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

}  // namespace dnfdaemon::client


#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_HPP
