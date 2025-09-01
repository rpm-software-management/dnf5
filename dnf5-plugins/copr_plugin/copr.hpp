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

#ifndef DNF5_COMMANDS_COPR_COPR_HPP
#define DNF5_COMMANDS_COPR_COPR_HPP

// TODO: implement 'dnf copr check' - This feature should try to compare the
//       currently installed repo files with the current Copr state (e.g.
//       projects removed in Copr, external requirements changed, multilib
//       changed, chroot EOL).
// TODO: implement 'dnf copr fix' - ^^^ should help

#include "copr_constants.hpp"
#include "helpers.hpp"

#include <dnf5/context.hpp>

#include <iostream>


namespace dnf5 {

using bool_opt_t = std::unique_ptr<libdnf5::cli::session::BoolOption>;
using str_map_t = std::map<std::string, std::string>;

class CoprCommand;

class CoprSubCommand : public Command {
public:
    explicit CoprSubCommand(Context & context, const std::string & name) : Command(context, name) {}
    CoprCommand * copr_cmd();
};


class CoprListCommand : public CoprSubCommand {
public:
    explicit CoprListCommand(Context & context) : CoprSubCommand(context, "list") {}
    void set_argument_parser() override;
    void run() override;

private:
    bool_opt_t installed{nullptr};
};


class CoprDebugCommand : public CoprSubCommand {
public:
    explicit CoprDebugCommand(Context & context) : CoprSubCommand(context, "debug") {}
    void set_argument_parser() override;
    void run() override;
};


class CoprSubCommandWithID : public CoprSubCommand {
private:
    std::string project_spec;

protected:
    std::string opt_hub = "";
    std::string opt_owner;
    std::string opt_dirname;

public:
    explicit CoprSubCommandWithID(Context & context, const std::string & name) : CoprSubCommand(context, name) {}
    void run() override;
    void set_argument_parser() override;

    /// Get the Copr Project specification we work with.  It can be either the
    /// shortcut 'owner/project' (hub resolves to default), or
    /// 'hub/owner/project'.  'hub' can be ID from the copr configuration
    /// ('fedora' stands for 'copr.fedorainfracloud.org') or the hostname
    /// directly.  The returned value is taken from the first positional
    /// argument (which is the Copr ID), but if no 'hub' is there, we take into
    /// account the value of --hub argument, too.
    std::string get_project_spec();
};


class CoprEnableCommand : public CoprSubCommandWithID {
private:
    std::string opt_chroot = "";

public:
    explicit CoprEnableCommand(Context & context) : CoprSubCommandWithID(context, "enable") {}
    void run() override;
    void set_argument_parser() override;
};


class CoprRemoveCommand : public CoprSubCommandWithID {
private:
    std::string opt_chroot = "";

public:
    explicit CoprRemoveCommand(Context & context) : CoprSubCommandWithID(context, "remove") {}
    void run() override;
    void set_argument_parser() override;
};


class CoprDisableCommand : public CoprSubCommandWithID {
public:
    explicit CoprDisableCommand(Context & context) : CoprSubCommandWithID(context, "disable") {}
    void run() override;
    void set_argument_parser() override;
};


class CoprCommand : public Command {
public:
    explicit CoprCommand(Context & context) : Command(context, "copr") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override { this->throw_missing_command(); };
    const std::string & hub() { return this->hub_option.get_value(); }

private:
    libdnf5::OptionString hub_option{""};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_COPR_COPR_HPP
