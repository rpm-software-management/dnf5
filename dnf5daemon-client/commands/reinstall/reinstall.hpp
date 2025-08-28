// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REINSTALL_REINSTALL_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REINSTALL_REINSTALL_HPP

#include "commands/command.hpp"

#include <libdnf5/conf/option_bool.hpp>

namespace dnfdaemon::client {

class ReinstallCommand : public TransactionCommand {
public:
    explicit ReinstallCommand(Context & context) : TransactionCommand(context, "reinstall") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs{};
    libdnf5::OptionBool offline_option{false};
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_COMMANDS_REINSTALL_REINSTALL_HPP
