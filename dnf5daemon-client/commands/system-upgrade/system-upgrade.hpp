// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_SYSTEM_UPGRADE_SYSTEM_UPGRADE_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_SYSTEM_UPGRADE_SYSTEM_UPGRADE_HPP

#include "commands/command.hpp"

#include <libdnf5/conf/option_bool.hpp>

namespace dnfdaemon::client {

class SystemUpgradeCommand : public TransactionCommand {
public:
    explicit SystemUpgradeCommand(Context & context) : TransactionCommand(context, "system-upgrade") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    dnfdaemon::KeyValueMap session_config() override;
    void run() override;

private:
    libdnf5::OptionBool no_downgrade_option{false};
};

}  // namespace dnfdaemon::client

#endif
