// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "advisory.hpp"

#include "advisory_info.hpp"
#include "advisory_list.hpp"
#include "advisory_summary.hpp"

namespace dnf5 {

void AdvisoryCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void AdvisoryCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Manage advisories"));
}

void AdvisoryCommand::register_subcommands() {
    auto * query_commands_advisory = get_context().get_argument_parser().add_new_group("advisory_query_commands");
    query_commands_advisory->set_header("Query Commands:");
    get_argument_parser_command()->register_group(query_commands_advisory);
    register_subcommand(std::make_unique<AdvisoryListCommand>(get_context()), query_commands_advisory);
    register_subcommand(std::make_unique<AdvisoryInfoCommand>(get_context()), query_commands_advisory);
    register_subcommand(std::make_unique<AdvisorySummaryCommand>(get_context()), query_commands_advisory);
}

void AdvisoryCommand::pre_configure() {
    throw_missing_command();
}

}  // namespace dnf5
