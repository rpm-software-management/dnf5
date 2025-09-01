// Copyright Contributors to the DNF5 project.
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

#ifndef DNF5DAEMON_CLIENT_COMMANDS_GROUP_GROUP_LIST_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_GROUP_GROUP_LIST_HPP

#include "commands/command.hpp"

#include <libdnf5-cli/session.hpp>
#include <libdnf5/conf/option_enum.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <memory>
#include <vector>

namespace dnfdaemon::client {

class GroupAvailableOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupAvailableOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "available", '\0', _("Show only available groups."), false) {}
};

class GroupInstalledOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupInstalledOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "installed", '\0', _("Show only installed groups."), false) {}
};

class GroupHiddenOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupHiddenOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "hidden", '\0', _("Show also hidden groups."), false) {}
};

class GroupContainPkgsOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit GroupContainPkgsOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "contains-pkgs",
              '\0',
              _("Show only groups containing packages with specified names. List option, supports globs."),
              "PACKAGE_NAME,...") {}
};


class GroupListCommand : public DaemonCommand {
public:
    explicit GroupListCommand(Context & context, const char * command);
    void run() override;

private:
    std::vector<std::unique_ptr<libdnf5::Option>> * patterns_options{nullptr};
    std::unique_ptr<GroupHiddenOption> hidden{nullptr};
    std::unique_ptr<GroupAvailableOption> available{nullptr};
    std::unique_ptr<GroupInstalledOption> installed{nullptr};
    std::unique_ptr<GroupContainPkgsOption> contains_pkgs{nullptr};
    const std::string command;
};

}  // namespace dnfdaemon::client

#endif
