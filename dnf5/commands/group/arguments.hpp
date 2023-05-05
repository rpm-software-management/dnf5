/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5_COMMANDS_GROUP_ARGUMENTS_HPP
#define DNF5_COMMANDS_GROUP_ARGUMENTS_HPP


#include "utils/bgettext/bgettext-lib.h"

#include <libdnf-cli/session.hpp>


namespace dnf5 {


class GroupAvailableOption : public libdnf::cli::session::BoolOption {
public:
    explicit GroupAvailableOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "available", '\0', _("Show only available groups."), false) {}
};


class GroupHiddenOption : public libdnf::cli::session::BoolOption {
public:
    explicit GroupHiddenOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "hidden", '\0', _("Show also hidden groups."), false) {}
};


class GroupInstalledOption : public libdnf::cli::session::BoolOption {
public:
    explicit GroupInstalledOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "installed", '\0', _("Show only installed groups."), false) {}
};


class GroupContainsPkgsOption : public libdnf::cli::session::AppendStringListOption {
public:
    explicit GroupContainsPkgsOption(libdnf::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "contains-pkgs",
              '\0',
              _("Show only groups containing packages with specified names. List option, supports globs."),
              "PACKAGE_NAME,...") {}
};


class GroupSpecArguments : public libdnf::cli::session::StringArgumentList {
public:
    GroupSpecArguments(libdnf::cli::session::Command & command, int nargs)
        : StringArgumentList(command, "group-spec", _("Pattern matching group IDS."), nargs) {}
    explicit GroupSpecArguments(libdnf::cli::session::Command & command)
        : GroupSpecArguments(command, libdnf::cli::ArgumentParser::PositionalArg::UNLIMITED) {}
};


class GroupWithOptionalOption : public libdnf::cli::session::BoolOption {
public:
    explicit GroupWithOptionalOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "with-optional", '\0', _("Include optional packages from group."), false) {}
};


class GroupNoPackagesOption : public libdnf::cli::session::BoolOption {
public:
    explicit GroupNoPackagesOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "no-packages", '\0', _("Operate on groups only, no packages are changed."), false) {}
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_ARGUMENTS_HPP
