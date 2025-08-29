// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REPO_REPO_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REPO_REPO_HPP

#include "commands/command.hpp"
#include "context.hpp"

namespace dnfdaemon::client {

class RepoCommand : public DaemonCommand {
public:
    explicit RepoCommand(Context & context) : DaemonCommand(context, "repo") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

}  // namespace dnfdaemon::client


#endif  // DNF5DAEMON_CLIENT_COMMANDS_REPO_REPO_HPP
