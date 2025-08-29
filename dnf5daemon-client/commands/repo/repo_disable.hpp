// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REPO_DISABLE_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REPO_DISABLE_HPP

#include "commands/command.hpp"
#include "repo_enable_disable.hpp"

namespace dnfdaemon::client {

class RepoDisableCommand : public RepoEnableDisableCommand {
public:
    explicit RepoDisableCommand(Context & context) : RepoEnableDisableCommand(context, "disable") {}
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_COMMANDS_REPO_DISABLE_HPP
