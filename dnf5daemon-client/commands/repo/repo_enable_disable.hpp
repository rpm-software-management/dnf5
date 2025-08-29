// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REPO_ENABLE_DISABLE_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REPO_ENABLE_DISABLE_HPP

#include "commands/command.hpp"

namespace dnfdaemon::client {

class RepoEnableDisableCommand : public DaemonCommand {
public:
    explicit RepoEnableDisableCommand(Context & context, const std::string & command)
        : DaemonCommand(context, command),
          command(command) {}
    void set_argument_parser() override;
    void run() override;

private:
    std::vector<std::unique_ptr<libdnf5::Option>> * patterns_options{nullptr};
    const std::string command;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_COMMANDS_REPO_ENABLE_DISABLE_HPP
