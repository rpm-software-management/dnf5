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
    /// Initialize dbus connection and server session
    void init_session();

    /// Updates the repositories metadata cache.
    /// Loads the updated metadata into rpm::RepoSack and into rpm::SolvSack.
    void load_rpm_repos(libdnf::rpm::RepoSet & repos, libdnf::rpm::SolvSack::LoadRepoFlags flags);

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
    std::unique_ptr<sdbus::IConnection> connection;
    sdbus::ObjectPath session_object_path;
};

}  // namespace dnfdaemon::client

#endif
