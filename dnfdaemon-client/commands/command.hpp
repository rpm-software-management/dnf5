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

#ifndef DNFDAEMON_CLIENT_COMMANDS_COMMAND_HPP
#define DNFDAEMON_CLIENT_COMMANDS_COMMAND_HPP

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf-cli/session.hpp>


namespace dnfdaemon::client {

class DaemonCommand : public libdnf::cli::session::Command {
public:
    explicit DaemonCommand(Command & parent, const std::string & name) : Command(parent, name){};
    explicit DaemonCommand(libdnf::cli::session::Session & session, const std::string & name)
        : Command(session, name){};
    virtual dnfdaemon::KeyValueMap session_config() {
        dnfdaemon::KeyValueMap cfg = {};
        return cfg;
    }
};

class TransactionCommand : public DaemonCommand {
public:
    explicit TransactionCommand(Command & parent, const std::string & name) : DaemonCommand(parent, name){};
    void run_transaction();
};


}  // namespace dnfdaemon::client

#endif
