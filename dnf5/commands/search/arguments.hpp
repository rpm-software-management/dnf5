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


#ifndef DNF5_COMMANDS_SEARCH_ARGUMENTS_HPP
#define DNF5_COMMANDS_SEARCH_ARGUMENTS_HPP


#include "dnf5/context.hpp"
#include "utils/bgettext/bgettext-lib.h"

#include <libdnf-cli/session.hpp>


namespace dnf5 {


class SearchAllOption : public libdnf::cli::session::BoolOption {
public:
    explicit SearchAllOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "all", '\0', _("Search also package description and URL."), false) {}
};


class SearchPatternsArguments : public libdnf::cli::session::StringArgumentList {
public:
    explicit SearchPatternsArguments(libdnf::cli::session::Command & command, Context & context)
        : StringArgumentList(
              command, "patterns", _("Patterns"), libdnf::cli::ArgumentParser::PositionalArg::AT_LEAST_ONE) {
        arg->set_complete_hook_func(
            [&context](const char * arg) { return match_specs(context, arg, true, true, false, false); });
    }
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_SEARCH_ARGUMENTS_HPP
