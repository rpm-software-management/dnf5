// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_INFO_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_INFO_HPP

#include "../../wrappers/dbus_advisory_wrapper.hpp"
#include "advisory_subcommand.hpp"

namespace dnfdaemon::client {

class AdvisoryInfoCommand : public AdvisorySubCommand {
public:
    explicit AdvisoryInfoCommand(Context & context) : AdvisorySubCommand(context, "info") {}

    void set_argument_parser() override {
        AdvisorySubCommand::set_argument_parser();
        get_argument_parser_command()->set_description(_("Print details about advisories"));
    }
    void pre_configure() override;

protected:
    void process_and_print_queries(const std::vector<DbusAdvisoryWrapper> & advisories) override;
};

}  // namespace dnfdaemon::client

#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_INFO_HPP
