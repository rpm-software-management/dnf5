// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "search.hpp"

#include "search_processor.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/output/search.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void SearchCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void SearchCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Search for software matching all specified strings"));

    all = std::make_unique<SearchAllOption>(*this);
    patterns = std::make_unique<SearchPatternsArguments>(*this, get_context());

    show_duplicates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "showduplicates", '\0', "Show all versions of the packages, not only the latest ones.", false);
}

void SearchCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void SearchCommand::run() {
    auto & base = get_context().get_base();
    SearchProcessor processor(base, patterns->get_value(), all->get_value(), show_duplicates->get_value());
    libdnf5::cli::output::print_search_results(processor.get_results());
}


}  // namespace dnf5
