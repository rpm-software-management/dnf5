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

#include "../../wrappers/dbus_advisory_wrapper.hpp"
#include "advisory_subcommand.hpp"

namespace dnfdaemon::client {

class AdvisoryListCommand : public AdvisorySubCommand {
public:
    explicit AdvisoryListCommand(Context & context) : AdvisorySubCommand(context, "list") {}

    void set_argument_parser() override {
        AdvisorySubCommand::set_argument_parser();
        get_argument_parser_command()->set_description(_("List advisories"));
    }
    void pre_configure() override;

protected:
    void process_and_print_queries(const std::vector<DbusAdvisoryWrapper> & advisories) override;
};

}  // namespace dnfdaemon::client

#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_LIST_HPP
