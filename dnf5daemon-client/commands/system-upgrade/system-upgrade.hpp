// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

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
