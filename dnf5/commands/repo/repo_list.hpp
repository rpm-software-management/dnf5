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


#ifndef DNF5_COMMANDS_REPO_REPO_LIST_HPP
#define DNF5_COMMANDS_REPO_REPO_LIST_HPP


#include "arguments.hpp"

#include <libdnf-cli/session.hpp>
#include <libdnf/conf/option_enum.hpp>
#include <libdnf/repo/repo_query.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class RepoListCommand : public libdnf::cli::session::Command {
public:
    explicit RepoListCommand(Command & parent);
    void run() override;
    virtual void print(const libdnf::repo::RepoQuery & query, bool with_status);

    std::unique_ptr<RepoAllOption> all{nullptr};
    std::unique_ptr<RepoEnabledOption> enabled{nullptr};
    std::unique_ptr<RepoDisabledOption> disabled{nullptr};
    std::unique_ptr<RepoSpecArguments> repo_specs{nullptr};

protected:
    // to be used by an alias command only
    explicit RepoListCommand(Command & parent, const std::string & name);
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_REPO_REPO_LIST_HPP
