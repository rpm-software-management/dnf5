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

#ifndef DNF5_COMMANDS_GROUP_ARGUMENTS_HPP
#define DNF5_COMMANDS_GROUP_ARGUMENTS_HPP


#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>


namespace dnf5 {


class GroupAvailableOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupAvailableOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "available", '\0', _("Show only available groups."), false) {}
};


class GroupHiddenOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupHiddenOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "hidden", '\0', _("Show also hidden groups."), false) {}
};


class GroupInstalledOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupInstalledOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "installed", '\0', _("Show only installed groups."), false) {}
};


class GroupContainsPkgsOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit GroupContainsPkgsOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "contains-pkgs",
              '\0',
              _("Show only groups containing packages with specified names. List option, supports globs."),
              "PACKAGE_NAME,...") {}
};


class GroupSpecArguments : public libdnf5::cli::session::StringArgumentList {
public:
    GroupSpecArguments(libdnf5::cli::session::Command & command, int nargs)
        : StringArgumentList(command, "group-spec", _("Pattern matching group identifiers."), nargs) {}
    explicit GroupSpecArguments(libdnf5::cli::session::Command & command)
        : GroupSpecArguments(command, libdnf5::cli::ArgumentParser::PositionalArg::UNLIMITED) {}
};


class CompsSpecArguments : public libdnf5::cli::session::StringArgumentList {
public:
    CompsSpecArguments(libdnf5::cli::session::Command & command, int nargs)
        : StringArgumentList(
              command, "group-spec|environment-spec", _("Pattern matching group or environment identifiers."), nargs) {}
    explicit CompsSpecArguments(libdnf5::cli::session::Command & command)
        : CompsSpecArguments(command, libdnf5::cli::ArgumentParser::PositionalArg::UNLIMITED) {}
};


class GroupWithOptionalOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupWithOptionalOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "with-optional", '\0', _("Include optional packages from groups."), false) {}
};


class GroupNoPackagesOption : public libdnf5::cli::session::BoolOption {
public:
    explicit GroupNoPackagesOption(libdnf5::cli::session::Command & command)
        : BoolOption(
              command,
              "no-packages",
              '\0',
              _("Operate on environments and groups only, no packages are changed."),
              false) {}
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_ARGUMENTS_HPP
