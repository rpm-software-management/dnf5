// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

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
