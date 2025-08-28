// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
    explicit SearchPatternsArguments(libdnf5::cli::session::Command & command, Context & context)
        : StringArgumentList(
              command, "patterns", _("Patterns"), libdnf5::cli::ArgumentParser::PositionalArg::AT_LEAST_ONE) {
        arg->set_complete_hook_func(
            [&context](const char * arg) { return match_specs(context, arg, true, true, false, false); });
    }
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_SEARCH_ARGUMENTS_HPP
