// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_UPGRADE_UPGRADE_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_UPGRADE_UPGRADE_HPP

#include "commands/command.hpp"

#include <libdnf5/conf/option_bool.hpp>

namespace dnfdaemon::client {

class UpgradeCommand : public TransactionCommand {
public:
    explicit UpgradeCommand(Context & context) : TransactionCommand(context, "upgrade") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs{};
    libdnf5::OptionBool offline_option{false};
};

}  // namespace dnfdaemon::client

#endif
