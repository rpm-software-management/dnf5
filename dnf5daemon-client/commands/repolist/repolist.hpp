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

#ifndef DNF5DAEMON_CLIENT_COMMANDS_REPOLIST_REPOLIST_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_REPOLIST_REPOLIST_HPP

#include "commands/command.hpp"

#include <libdnf5/conf/option.hpp>
#include <libdnf5/conf/option_enum.hpp>

#include <memory>
#include <string>
#include <vector>

namespace dnfdaemon::client {

class RepolistCommand : public DaemonCommand {
public:
    explicit RepolistCommand(Context & context, const char * command)
        : DaemonCommand(context, command),
          command(command) {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

private:
    libdnf5::OptionEnum * enable_disable_option{nullptr};
    std::vector<std::unique_ptr<libdnf5::Option>> * patterns_options{nullptr};
    const std::string command;
};

}  // namespace dnfdaemon::client

#endif
