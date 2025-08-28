// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REPO_ENABLE_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REPO_ENABLE_HPP

#include "commands/command.hpp"
#include "repo_enable_disable.hpp"

namespace dnfdaemon::client {

class RepoEnableCommand : public RepoEnableDisableCommand {
public:
    explicit RepoEnableCommand(Context & context) : RepoEnableDisableCommand(context, "enable") {}
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_COMMANDS_REPO_ENABLE_HPP
