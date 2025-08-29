// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_SUMMARY_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_SUMMARY_HPP

#include "advisory_subcommand.hpp"

namespace dnf5 {

class AdvisorySummaryCommand : public AdvisorySubCommand {
public:
    explicit AdvisorySummaryCommand(Context & context) : AdvisorySubCommand(context, "summary") {}

    void set_argument_parser() override {
        AdvisorySubCommand::set_argument_parser();
        get_argument_parser_command()->set_description(_("Print summary of advisories"));
    }

protected:
    void process_and_print_queries(
        Context & ctx,
        libdnf5::advisory::AdvisoryQuery & advisories,
        const std::vector<std::string> & package_specs) override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_SUMMARY_HPP
