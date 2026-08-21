// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_COMMANDS_NO_MATCHES_HPP
#define DNF5_COMMANDS_NO_MATCHES_HPP


#include <dnf5/context.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <set>
#include <string>
#include <string_view>


namespace dnf5 {

// Checks whether there are any enabled repositories to query at all.
bool no_repos_enabled(Context & ctx);

// Reports why there are no repositories to query: an installroot hint when
// "--installroot" is used without "--use-host-config" and no repositories
// were configured inside the installroot at all, or otherwise a generic
// message (if given).
// @param generic_message Message to print when the installroot hint does not
//                         apply. Nothing is printed if left empty.
// @return `true` if the installroot hint was printed, `false` otherwise.
bool report_no_repos(Context & ctx, std::string_view generic_message = {});

// Reports unmatched specs (partial match) or why the output is empty.
//
// @param unmatched_specs       Specs that did not match anything.
// @param result_is_empty       Whether the final result set is empty.
// @param no_repos              No enabled repositories to query. Pass `false`
//                               when the query does not depend on enabled
//                               repositories (e.g. --installed, --disabled).
// @param no_candidates         No candidates exist before spec filtering.
// @param no_candidates_message Domain-specific message for the no_candidates case.
void report_no_matches(
    Context & ctx,
    const std::set<std::string> & unmatched_specs,
    bool result_is_empty,
    bool no_repos,
    bool no_candidates,
    std::string_view no_candidates_message = _("No matches found."));

}  // namespace dnf5


#endif  // DNF5_COMMANDS_NO_MATCHES_HPP
