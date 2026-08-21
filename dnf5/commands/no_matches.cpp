// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "no_matches.hpp"

#include <libdnf5/repo/repo_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

namespace {

void report_unmatched_specs(dnf5::Context & ctx, const std::set<std::string> & unmatched_specs) {
    for (const auto & spec : unmatched_specs) {
        ctx.print_info(libdnf5::utils::sformat(_("No matches found for \"{}\"."), spec));
    }
}

}  // namespace

namespace dnf5 {

bool no_repos_enabled(Context & ctx) {
    libdnf5::repo::RepoQuery enabled_repos(ctx.get_base());
    enabled_repos.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);
    enabled_repos.filter_enabled(true);
    return enabled_repos.empty();
}

bool report_no_repos(Context & ctx, std::string_view generic_message) {
    auto & config = ctx.get_base().get_config();
    if (config.get_installroot_option().get_value() != "/" && !config.get_use_host_config_option().get_value()) {
        libdnf5::repo::RepoQuery all_repos(ctx.get_base());
        all_repos.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);
        if (all_repos.empty()) {
            ctx.print_info(
                _("No repositories were loaded from the installroot. To use the configuration and "
                  "repositories of the host system, pass --use-host-config."));
            return true;
        }
    }
    if (!generic_message.empty()) {
        ctx.print_info(generic_message);
    }
    return false;
}

void report_no_matches(
    Context & ctx,
    const std::set<std::string> & unmatched_specs,
    bool result_is_empty,
    bool no_repos,
    bool no_candidates,
    std::string_view no_candidates_message) {
    if (!result_is_empty) {
        if (!unmatched_specs.empty()) {
            report_unmatched_specs(ctx, unmatched_specs);
        }
        return;
    }
    if (no_repos) {
        report_no_repos(ctx, _("No matches found: no repositories are enabled."));
    } else if (no_candidates) {
        ctx.print_info(no_candidates_message);
    } else {
        ctx.print_info(_("No matches found."));
    }
}

}  // namespace dnf5
