// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "autoremove.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/rpm/package_query.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void AutoremoveCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void AutoremoveCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(
        _("Remove all unneeded packages originally installed as dependencies."));
    create_offline_option(*this);
    create_store_option(*this);
}

void AutoremoveCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void AutoremoveCommand::run() {
    auto & ctx = get_context();
    libdnf5::rpm::PackageQuery unneeded(ctx.get_base());
    unneeded.filter_unneeded();
    auto goal = get_context().get_goal();
    for (const auto & pkg : unneeded) {
        goal->add_rpm_remove(pkg);
    }
}

}  // namespace dnf5
