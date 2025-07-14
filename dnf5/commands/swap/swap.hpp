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

#ifndef DNF5_COMMANDS_SWAP_SWAP_HPP
#define DNF5_COMMANDS_SWAP_SWAP_HPP

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>

#include <vector>

namespace dnf5 {

// TODO(jrohel): The "swap" command may be removed in the future in favor of a more powerful command (eg "do"),
//               which will allow multiple actions to be combined in one transaction.
class SwapCommand : public Command {
public:
    explicit SwapCommand(Context & context) : Command(context, "swap") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::string remove_pkg_spec;
    std::string install_pkg_spec;

    std::unique_ptr<AllowErasingOption> allow_erasing;

    std::vector<std::string> installed_from_repos;
    std::vector<std::string> from_repos;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_SWAP_SWAP_HPP
