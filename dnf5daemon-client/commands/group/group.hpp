// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_GROUP_GROUP_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_GROUP_GROUP_HPP

#include "commands/command.hpp"

#include <libdnf5/conf/option_enum.hpp>

#include <memory>
#include <vector>

namespace dnfdaemon::client {

class GroupCommand : public DaemonCommand {
public:
    explicit GroupCommand(Context & context) : DaemonCommand(context, "group") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void pre_configure() override;
};

}  // namespace dnfdaemon::client

#endif
