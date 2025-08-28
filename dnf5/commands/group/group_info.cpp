// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "group_info.hpp"

#include <libdnf5-cli/output/adapters/comps.hpp>
#include <libdnf5-cli/output/groupinfo.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void GroupInfoCommand::print(const libdnf5::comps::GroupQuery & query) {
    std::vector<libdnf5::comps::Group> groups(query.list().begin(), query.list().end());
    std::sort(groups.begin(), groups.end(), libdnf5::cli::output::comps_display_order_cmp<libdnf5::comps::Group>);

    for (auto group : groups) {
        libdnf5::cli::output::GroupAdapter cli_group(group);
        libdnf5::cli::output::print_groupinfo_table(cli_group);
        std::cout << '\n';
    }
}

}  // namespace dnf5
