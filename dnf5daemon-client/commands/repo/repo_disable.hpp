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

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REPO_DISABLE_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REPO_DISABLE_HPP

#include "commands/command.hpp"
#include "repo_enable_disable.hpp"

namespace dnfdaemon::client {

class RepoDisableCommand : public RepoEnableDisableCommand {
public:
    explicit RepoDisableCommand(Context & context) : RepoEnableDisableCommand(context, "disable") {}
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_COMMANDS_REPO_DISABLE_HPP
