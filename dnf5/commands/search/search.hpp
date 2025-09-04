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


#ifndef DNF5_COMMANDS_SEARCH_SEARCH_HPP
#define DNF5_COMMANDS_SEARCH_SEARCH_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <libdnf5-cli/session.hpp>

#include <memory>
#include <vector>


namespace dnf5 {

class SearchCommand : public Command {
public:
    explicit SearchCommand(Context & context) : Command(context, "search") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::unique_ptr<SearchAllOption> all{nullptr};
    std::unique_ptr<SearchPatternsArguments> patterns{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> show_duplicates{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> search_name{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> search_summary{nullptr};
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_SEARCH_SEARCH_HPP
