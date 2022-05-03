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


class GroupSpecArguments : public libdnf::cli::session::StringArgumentList {
public:
    explicit GroupSpecArguments(libdnf::cli::session::Command & command)
        : StringArgumentList(command, "group-spec", _("Pattern matching group IDs.")) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_ARGUMENTS_HPP
