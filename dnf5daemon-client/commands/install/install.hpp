/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5DAEMON_CLIENT_COMMANDS_INSTALL_INSTALL_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_INSTALL_INSTALL_HPP

#include "commands/command.hpp"

#include <libdnf/conf/option.hpp>
#include <libdnf/conf/option_bool.hpp>

namespace dnfdaemon::client {

class InstallCommand : public TransactionCommand {
public:
    explicit InstallCommand(Context & context) : TransactionCommand(context, "install") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

private:
    libdnf::OptionBool skip_broken_option{false};
    libdnf::OptionBool skip_unavailable_option{false};
    std::vector<std::unique_ptr<libdnf::Option>> * patterns_options{nullptr};
};

}  // namespace dnfdaemon::client

#endif
