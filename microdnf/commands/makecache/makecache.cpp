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


#include "makecache.hpp"

#include "microdnf/context.hpp"

#include <fmt/format.h>

#include <iostream>


namespace microdnf {


using namespace libdnf::cli;


MakeCacheCommand::MakeCacheCommand(Command & parent) : Command(parent, "makecache") {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Generate the metadada cache");
}


void MakeCacheCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    libdnf::repo::RepoQuery enabled_repos_query(ctx.base);
    enabled_repos_query.filter_enabled(true);
    if (enabled_repos_query.empty()) {
        std::string repos_paths;
        bool first = true;
        for (const auto & val : ctx.base.get_config().reposdir().get_value()) {
            if (!first) {
                repos_paths += ", ";
            }
            repos_paths += '"' + val + '"';
            first = false;
        }
        std::cout << fmt::format("There are no enabled repositories in {}.", repos_paths) << std::endl;
        return;
    }

    ctx.load_repos(false);

    std::cout << "Metadata cache created." << std::endl;
}


}  // namespace microdnf
