/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_CLIENT_CONTEXT_HPP
#define DNFDAEMON_CLIENT_CONTEXT_HPP

#include "argument_parser.hpp"
#include "commands/command.hpp"

#include <libdnf/base/base.hpp>
#include <libdnf/rpm/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <memory>
#include <utility>
#include <vector>

// TODO(mblaha) remove after microdnf is merged
namespace libdnf::rpm {
using RepoWeakPtr = WeakPtr<Repo, false>;
using RepoSet = Set<RepoWeakPtr>;
}

namespace dnfdaemon::client {

constexpr const char * VERSION = "0.1.0";

class Context {
public:
    enum class RepoStatus {NOT_READY, PENDING, READY, ERROR};
    Context(sdbus::IConnection & connection) : connection(connection), repositories_status(RepoStatus::NOT_READY) {};

    /// Initialize dbus connection and server session
    void init_session();

    // initialize repository metadata loading on server side and wait for results
    RepoStatus wait_for_repos();

    // signal handlers
    void on_repositories_ready(const bool & result);

    /// Select command to execute
    void select_command(Command * cmd) { selected_command = cmd; }

    std::vector<std::pair<std::string, std::string>> setopts;
    std::vector<std::unique_ptr<Command>> commands;
    Command * selected_command{nullptr};
    ArgumentParser arg_parser;
    /// proxy to dnfdaemon session
    std::unique_ptr<sdbus::IProxy> session_proxy;

private:
    /// system d-bus connection
    //std::unique_ptr<sdbus::IConnection> connection;
    sdbus::IConnection & connection;
    sdbus::ObjectPath session_object_path;
    RepoStatus repositories_status;
};

}  // namespace dnfdaemon::client

#endif
