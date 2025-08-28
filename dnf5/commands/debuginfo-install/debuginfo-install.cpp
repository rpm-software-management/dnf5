// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "debuginfo-install.hpp"

#include <libdnf5/conf/option_string.hpp>

#include <memory>


namespace dnf5 {


using namespace libdnf5::cli;


void DebuginfoInstallCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void DebuginfoInstallCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    get_argument_parser_command()->set_description(_("Install debuginfo packages."));

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);

    patterns_to_debuginfo_install_options = parser.add_new_values();
    auto patterns_arg = parser.add_new_positional_arg(
        "package-spec-NPFB",
        ArgumentParser::PositionalArg::AT_LEAST_ONE,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        patterns_to_debuginfo_install_options);
    patterns_arg->set_description("List of package-spec-NPFB to install the associated debuginfo packages for");
    cmd.register_positional_arg(patterns_arg);
    create_offline_option(*this);
    create_store_option(*this);
}

void DebuginfoInstallCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.get_base().get_repo_sack()->enable_debug_repos();
}

void DebuginfoInstallCommand::run() {
    auto goal = get_context().get_goal();
    auto settings = libdnf5::GoalJobSettings();
    goal->set_allow_erasing(allow_erasing->get_value());

    for (const auto & pattern : *patterns_to_debuginfo_install_options) {
        auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
        goal->add_debug_install(option->get_value(), settings);
    }
}

}  // namespace dnf5
