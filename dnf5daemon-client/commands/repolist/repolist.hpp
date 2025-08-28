// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REPOLIST_REPOLIST_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REPOLIST_REPOLIST_HPP

#include "commands/command.hpp"

#include <libdnf5/conf/option.hpp>
#include <libdnf5/conf/option_enum.hpp>

#include <memory>
#include <string>
#include <vector>

namespace dnfdaemon::client {

class RepolistCommand : public DaemonCommand {
public:
    explicit RepolistCommand(Context & context, const char * command)
        : DaemonCommand(context, command),
          command(command) {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

private:
    libdnf5::OptionEnum * enable_disable_option{nullptr};
    std::vector<std::unique_ptr<libdnf5::Option>> * patterns_options{nullptr};
    const std::string command;
};

}  // namespace dnfdaemon::client

#endif
