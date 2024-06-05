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

#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_LIST_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_LIST_HPP

#include "advisory_subcommand.hpp"

#include <dnf5/shared_options.hpp>

namespace dnf5 {

class AdvisoryListCommand : public AdvisorySubCommand {
public:
    explicit AdvisoryListCommand(Context & context) : AdvisorySubCommand(context, "list") {}

    void set_argument_parser() override {
        AdvisorySubCommand::set_argument_parser();
        get_argument_parser_command()->set_description(_("List advisories"));
        create_json_option(*this);
    }

protected:
    void process_and_print_queries(
        Context & ctx,
        libdnf5::advisory::AdvisoryQuery & advisories,
        const std::vector<std::string> & package_specs) override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_LIST_HPP
