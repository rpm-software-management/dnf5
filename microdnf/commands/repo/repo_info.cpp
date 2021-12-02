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


#include "repo_info.hpp"

#include "context.hpp"

#include <libdnf/conf/option_string.hpp>
#include "libdnf-cli/output/repo_info.hpp"

#include <iostream>


namespace microdnf {


using namespace libdnf::cli;


RepoInfoCommand::RepoInfoCommand(Command & parent) : RepoInfoCommand(parent, "info") {}


RepoInfoCommand::RepoInfoCommand(Command & parent, const std::string & name) : RepoListCommand(parent, name) {}


void RepoInfoCommand::print(const libdnf::repo::RepoQuery & query, [[maybe_unused]] bool with_status) {
    for (auto & repo : query.get_data()) {
        libdnf::cli::output::RepoInfo repo_info;
        repo_info.add_repo(*repo, false, false);
        repo_info.print();
        std::cout << std::endl;
    }
}


}  // namespace microdnf
