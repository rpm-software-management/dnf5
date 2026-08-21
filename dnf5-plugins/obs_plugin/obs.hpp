// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_COMMANDS_OBS_OBS_HPP
#define DNF5_COMMANDS_OBS_OBS_HPP

#include "obs_constants.hpp"

#include <dnf5/context.hpp>

#include <iostream>


namespace dnf5 {

class ObsCommand;

class ObsSubCommand : public Command {
public:
    explicit ObsSubCommand(Context & context, const std::string & name) : Command(context, name) {}
    ObsCommand * obs_cmd();
};


class ObsListCommand : public ObsSubCommand {
public:
    explicit ObsListCommand(Context & context) : ObsSubCommand(context, "list") {}
    void set_argument_parser() override;
    void run() override;
};


class ObsDebugCommand : public ObsSubCommand {
public:
    explicit ObsDebugCommand(Context & context) : ObsSubCommand(context, "debug") {}
    void set_argument_parser() override;
    void run() override;
};


class ObsSubCommandWithID : public ObsSubCommand {
private:
    std::string project_repo_spec;

protected:
    std::string opt_hub = "";
    std::string opt_project;
    std::string opt_reponame;

public:
    explicit ObsSubCommandWithID(Context & context, const std::string & name) : ObsSubCommand(context, name) {}
    void run() override;
    void set_argument_parser() override;

    /// Get the OBS Project Repository specification we work with.  It can be
    /// either the shortcut 'project/reponame' (hub resolves to default), or
    /// 'hub/project/reponame'.  'hub' can be ID from the obs configuration
    /// ('opensuse' stands for 'build.opensuse.org') or the hostname
    /// directly.  The returned value is taken from the first positional
    /// argument, but if no 'hub' is there, we take into account the value
    /// of --hub argument, too.
    std::string get_project_repo_spec();
};


class ObsEnableCommand : public ObsSubCommandWithID {
public:
    explicit ObsEnableCommand(Context & context) : ObsSubCommandWithID(context, "enable") {}
    void run() override;
    void set_argument_parser() override;
};


class ObsRemoveCommand : public ObsSubCommandWithID {
public:
    explicit ObsRemoveCommand(Context & context) : ObsSubCommandWithID(context, "remove") {}
    void run() override;
    void set_argument_parser() override;
};


class ObsDisableCommand : public ObsSubCommandWithID {
public:
    explicit ObsDisableCommand(Context & context) : ObsSubCommandWithID(context, "disable") {}
    void run() override;
    void set_argument_parser() override;
};


class ObsCommand : public Command {
public:
    explicit ObsCommand(Context & context) : Command(context, "obs") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override { this->throw_missing_command(); };
    const std::string & hub() { return this->hub_option.get_value(); }

private:
    libdnf5::OptionString hub_option{""};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_OBS_OBS_HPP
