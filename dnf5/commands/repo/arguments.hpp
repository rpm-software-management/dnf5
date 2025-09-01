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


#ifndef DNF5_COMMANDS_REPO_ARGUMENTS_HPP
#define DNF5_COMMANDS_REPO_ARGUMENTS_HPP


#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>


namespace dnf5 {


class RepoAllOption : public libdnf5::cli::session::BoolOption {
public:
    explicit RepoAllOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "all", '\0', _("Show all repositories."), false) {}
};


class RepoEnabledOption : public libdnf5::cli::session::BoolOption {
public:
    explicit RepoEnabledOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "enabled", '\0', _("Show enabled repositories (default)."), false) {}
};


class RepoDisabledOption : public libdnf5::cli::session::BoolOption {
public:
    explicit RepoDisabledOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "disabled", '\0', _("Show disabled repositories."), false) {}
};


class RepoSpecArguments : public libdnf5::cli::session::StringArgumentList {
public:
    explicit RepoSpecArguments(libdnf5::cli::session::Command & command)
        : StringArgumentList(command, "repo-spec", _("Pattern matching repo IDs.")) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_REPO_ARGUMENTS_HPP
