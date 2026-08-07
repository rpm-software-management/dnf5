// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "group_summary.hpp"

#include <libdnf5-cli/output/groupsummary.hpp>

namespace dnf5 {

void GroupSummaryCommand::set_argument_parser() {
    GroupListCommand::set_argument_parser();
    get_argument_parser_command()->set_description(
        "Display summary statistics about groups, packages and repositories");
}

void GroupSummaryCommand::print(const libdnf5::comps::GroupQuery & query) {
    libdnf5::cli::output::print_groupsummary_table(get_context().get_base(), query);
}

}  // namespace dnf5
