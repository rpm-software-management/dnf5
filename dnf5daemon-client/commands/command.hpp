// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_COMMAND_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_COMMAND_HPP

#include "context.hpp"

#include <dnf5daemon-server/dbus.hpp>


namespace dnfdaemon::client {

class DaemonCommand : public libdnf5::cli::session::Command {
public:
    explicit DaemonCommand(Context & context, const std::string & name) : Command(context, name) {};
    virtual dnfdaemon::KeyValueMap session_config() {
        dnfdaemon::KeyValueMap cfg = {};
        return cfg;
    }

    /// @return Reference to the Context.
    Context & get_context() const noexcept { return static_cast<Context &>(get_session()); }
};

class TransactionCommand : public DaemonCommand {
public:
    explicit TransactionCommand(Context & context, const std::string & name) : DaemonCommand(context, name) {};
    void run_transaction(bool offline = false);
};


}  // namespace dnfdaemon::client

#endif
