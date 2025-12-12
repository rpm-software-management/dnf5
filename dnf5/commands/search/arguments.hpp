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


#ifndef DNF5_COMMANDS_SEARCH_ARGUMENTS_HPP
#define DNF5_COMMANDS_SEARCH_ARGUMENTS_HPP


#include "dnf5/context.hpp"

#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>


namespace dnf5 {


class SearchAllOption : public libdnf5::cli::session::BoolOption {
public:
    explicit SearchAllOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "all", '\0', _("Search also package description and URL."), false) {}
};


class SearchPatternsArguments : public libdnf5::cli::session::StringArgumentList {
public:
    explicit SearchPatternsArguments(libdnf5::cli::session::Command & command, Context & ctx)
        : StringArgumentList(
              command, "patterns", _("Patterns"), libdnf5::cli::ArgumentParser::PositionalArg::AT_LEAST_ONE) {
        arg->set_complete_hook_func(
            [&ctx](const char * arg) { return ctx.match_specs(arg, true, true, false, false); });
    }
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_SEARCH_ARGUMENTS_HPP
