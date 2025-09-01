// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "system-upgrade.hpp"

#include "../offline/offline.hpp"

#include "libdnf5/comps/environment/query.hpp"
#include "libdnf5/comps/group/query.hpp"
#include "libdnf5/utils/bgettext/bgettext-lib.h"

#include <libdnf5-cli/output/transaction_table.hpp>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_path.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>
#include <sys/wait.h>

#include <iostream>
#include <string>

using namespace libdnf5::cli;

namespace dnf5 {

void SystemUpgradeCommand::pre_configure() {
    throw_missing_command();
}

void SystemUpgradeCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void SystemUpgradeCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Prepare system for upgrade to a new release"));
}

void SystemUpgradeCommand::register_subcommands() {
    register_subcommand(std::make_unique<OfflineCleanCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineLogCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineRebootCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineStatusCommand>(get_context()));
    register_subcommand(std::make_unique<SystemUpgradeDownloadCommand>(get_context()));
}

void SystemUpgradeDownloadCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(_("Download everything needed to upgrade to a new release"));

    no_downgrade =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));

    auto * no_downgrade_arg = parser.add_new_named_arg("no-downgrade");
    no_downgrade_arg->set_long_name("no-downgrade");
    no_downgrade_arg->set_description(
        _("Do not install packages from the new release if they are older than what is currently installed"));
    no_downgrade_arg->set_const_value("true");
    no_downgrade_arg->link_value(no_downgrade);
    cmd.register_named_arg(no_downgrade_arg);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
}

void SystemUpgradeDownloadCommand::configure() {
    auto & ctx = get_context();

    const std::filesystem::path installroot{ctx.get_base().get_config().get_installroot_option().get_value()};

    target_releasever = ctx.get_base().get_vars()->get_value("releasever");

    const auto & detected_releasever = libdnf5::Vars::detect_release(ctx.get_base().get_weak_ptr(), installroot);
    if (detected_releasever != nullptr) {
        system_releasever = *detected_releasever;

        // Check --releasever
        if (target_releasever == system_releasever) {
            throw libdnf5::cli::CommandExitError(1, M_("Need a --releasever greater than the current system version."));
        }
    }

    ctx.set_load_system_repo(true);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    ctx.get_base().get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_COMPS);
}

void SystemUpgradeDownloadCommand::run() {
    auto & ctx = get_context();

    const auto & goal = ctx.get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());

    if (no_downgrade->get_value()) {
        goal->add_rpm_upgrade();
    } else {
        goal->add_rpm_distro_sync();
    }

    libdnf5::comps::GroupQuery q_groups(ctx.get_base());
    q_groups.filter_installed(true);
    for (const auto & grp : q_groups) {
        goal->add_group_upgrade(grp.get_groupid());
    }

    libdnf5::comps::EnvironmentQuery q_environments(ctx.get_base());
    q_environments.filter_installed(true);
    for (const auto & env : q_environments) {
        goal->add_group_upgrade(env.get_environmentid());
    }

    ctx.set_should_store_offline(true);
}

void OfflineDistroSyncCommand::pre_configure() {
    throw_missing_command();
}

void OfflineDistroSyncCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void OfflineDistroSyncCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Store a distro-sync transaction to be performed offline"));
}

void OfflineDistroSyncCommand::register_subcommands() {
    register_subcommand(std::make_unique<OfflineCleanCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineRebootCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineLogCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineStatusCommand>(get_context()));
}

void OfflineUpgradeCommand::pre_configure() {
    throw_missing_command();
}

void OfflineUpgradeCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void OfflineUpgradeCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Store an upgrade transaction to be performed offline"));
}

void OfflineUpgradeCommand::register_subcommands() {
    register_subcommand(std::make_unique<OfflineCleanCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineRebootCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineLogCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineStatusCommand>(get_context()));
}

}  // namespace dnf5
