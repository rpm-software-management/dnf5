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

#include "makecache.hpp"

#include <dnf5/shared_options.hpp>
#include <fmt/format.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void MakeCacheCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void MakeCacheCommand::set_argument_parser() {
    get_argument_parser_command()->set_description("Generate the metadata cache");
}

void MakeCacheCommand::configure() {
    auto & ctx = get_context();
    ctx.set_load_system_repo(false);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    libdnf5::repo::RepoQuery enabled_repos_query(ctx.get_base());
    enabled_repos_query.filter_enabled(true);
    if (enabled_repos_query.empty()) {
        std::string repos_paths;
        bool first = true;
        for (const auto & val : ctx.get_base().get_config().get_reposdir_option().get_value()) {
            if (!first) {
                repos_paths += ", ";
            }
            repos_paths += '"' + val + '"';
            first = false;
        }
        std::cout << fmt::format("There are no enabled repositories in {}.", repos_paths) << std::endl;
        return;
    }
}

void MakeCacheCommand::run() {
    std::cout << "Metadata cache created." << std::endl;
}

}  // namespace dnf5
