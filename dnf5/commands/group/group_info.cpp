/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "group_info.hpp"

#include <libdnf5-cli/output/adapters/comps.hpp>
#include <libdnf5-cli/output/groupinfo.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void GroupInfoCommand::print(const libdnf5::comps::GroupQuery & query) {
    std::vector<libdnf5::comps::Group> groups(query.list().begin(), query.list().end());
    std::sort(groups.begin(), groups.end(), libdnf5::comps::group_display_order_cmp);

    for (auto group : groups) {
        libdnf5::cli::output::GroupAdapter cli_group(group);
        libdnf5::cli::output::print_groupinfo_table(cli_group);
        std::cout << '\n';
    }
}

}  // namespace dnf5
