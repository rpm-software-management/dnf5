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

#include "leaves.hpp"

#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void LeavesCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void LeavesCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(
        "List groups of installed packages not required by other installed packages");
    get_argument_parser_command()->set_long_description(
        R"(The `leaves` command is used to list all leaf packages.

Leaf packages are installed packages that are not required as a dependency
of another installed package. However, two or more installed packages might
depend on each other in a dependency cycle. Packages in such cycles that
are not required by any other installed package are also leaf.
Packages in such cycles form a group of leaf packages.

Packages in the output list are sorted by group and the first package
in the group is preceded by a '-' character.)");
}

void LeavesCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void LeavesCommand::run() {
    auto & ctx = get_context();

    libdnf5::rpm::PackageQuery leaves_package_query(ctx.base, libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);
    auto leaves_package_groups = leaves_package_query.filter_leaves_groups();

    for (auto & package_group : leaves_package_groups) {
        std::sort(package_group.begin(), package_group.end());
    }
    std::sort(
        leaves_package_groups.begin(),
        leaves_package_groups.end(),
        [](const std::vector<libdnf5::rpm::Package> & a, const std::vector<libdnf5::rpm::Package> & b) {
            return a[0] < b[0];
        });

    // print the packages grouped by their components
    for (const auto & package_group : leaves_package_groups) {
        char mark = '-';

        for (const auto & package : package_group) {
            std::cout << mark << ' ' << package.get_full_nevra() << '\n';
            mark = ' ';
        }
    }
}

}  // namespace dnf5
