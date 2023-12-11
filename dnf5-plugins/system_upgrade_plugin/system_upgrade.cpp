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

#include "system_upgrade.hpp"

#include <libdnf5/base/goal.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_path.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <toml.hpp>

#include <iostream>

/* State::State(std::filesystem::path path) : path(std::move(path)) { */
/*     read(); */
/* } */
/* void State::read() { */
/*     auto toml_value = toml::parse(path); */
/* } */
/* void State::write() { */

/* } */

namespace dnf5 {

using namespace libdnf5::cli;

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
    get_argument_parser_command()->set_description("Prepare system for upgrade to a new release");
}

void SystemUpgradeCommand::register_subcommands() {
    register_subcommand(std::make_unique<SystemUpgradeDownloadCommand>(get_context()));
    register_subcommand(std::make_unique<SystemUpgradeUpgradeCommand>(get_context()));
}

SystemUpgradeSubcommand::SystemUpgradeSubcommand(Context & context, const std::string & name) : Command(context, name) {
    data_dir = std::filesystem::path(libdnf5::SYSTEM_STATE_DIR) / "system-upgrade";
}

void SystemUpgradeSubcommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    download_dir =
        dynamic_cast<libdnf5::OptionPath *>(parser.add_init_value(std::make_unique<libdnf5::OptionPath>(data_dir)));

    auto * download_dir_arg = parser.add_new_named_arg("downloaddir");
    download_dir_arg->set_long_name("downloaddir");
    download_dir_arg->set_description("Redirect download of packages to provided <path>");
    download_dir_arg->link_value(download_dir);
    cmd.register_named_arg(download_dir_arg);
}

void SystemUpgradeSubcommand::configure() {
    auto & ctx = get_context();
    const std::filesystem::path installroot{ctx.base.get_config().get_installroot_option().get_value()};
    magic_symlink = installroot / "system-update";
}

void SystemUpgradeDownloadCommand::set_argument_parser() {
    SystemUpgradeSubcommand::set_argument_parser();

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("Downloads everything needed to upgrade to a new release");

    no_downgrade =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(true)));

    auto * no_downgrade_arg = parser.add_new_named_arg("no-downgrade");
    no_downgrade_arg->set_long_name("no-downgrade");
    no_downgrade_arg->set_description(
        "Do not install packages from the new release if they are older than what is currently installed");
    no_downgrade_arg->link_value(no_downgrade);

    cmd.register_named_arg(no_downgrade_arg);
}

void SystemUpgradeDownloadCommand::configure() {
    SystemUpgradeSubcommand::configure();

    auto & ctx = get_context();
    auto & conf = ctx.base.get_config();

    // Check --releasever
    const auto & installroot = conf.get_installroot_option().get_value();
    const auto current_release = *libdnf5::Vars::detect_release(ctx.base.get_weak_ptr(), installroot);
    /* const auto & desired_release = ctx.base.get_vars()->get_value("releasever"); */

    // TODO: uncomment this. Easier to test with it commented out.
    /* if (desired_release == current_release) { */
    /*     throw libdnf5::cli::CommandExitError(1, M_("Need a --releasever greater than the current system version.")); */
    /* } */

    ctx.set_load_system_repo(true);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    ctx.base.get_config().get_cachedir_option().set(
        libdnf5::Option::Priority::PLUGINDEFAULT, get_download_dir()->get_value());
}

void SystemUpgradeDownloadCommand::run() {
    auto & ctx = get_context();

    const auto & goal = std::make_unique<libdnf5::Goal>(ctx.base);

    auto & conf = ctx.base.get_config();
    conf.opt_binds().at("tsflags").new_string(libdnf5::Option::Priority::RUNTIME, "test");

    if (no_downgrade->get_value()) {
        const auto & settings = libdnf5::GoalJobSettings();
        goal->add_rpm_upgrade(settings);
    } else {
        goal->add_rpm_distro_sync();
    }

    auto transaction = goal->resolve();
    if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw libdnf5::cli::GoalResolveError(transaction);
    }

    ctx.download_and_run(transaction);

    const std::string json = transaction.serialize();
    if (json.empty()) {
        throw libdnf5::cli::CommandExitError(1, M_("Transaction is empty."));
    }

    const auto & transaction_json_path = std::filesystem::path(get_data_dir()) / "system-upgrade-transaction.json";
    auto transaction_json_file = libdnf5::utils::fs::File(transaction_json_path, "w");

    transaction_json_file.write(json);
    transaction_json_file.close();
}

void reboot(Context & ctx, bool poweroff = false) {
    const std::string systemd_destination_name{"org.freedesktop.systemd1"};
    const std::string systemd_object_path{"/org/freedesktop/systemd1"};
    const std::string systemd_manager_interface{"org.freedesktop.systemd1.Manager"};

    if (std::getenv("DNF_SYSTEM_UPGRADE_NO_REBOOT")) {
        ctx.base.get_logger()->info("Reboot turned off, not rebooting.");
        return;
    }

    std::unique_ptr<sdbus::IConnection> connection;
    try {
        connection = sdbus::createSystemBusConnection();
    } catch (const sdbus::Error & ex) {
        const std::string error_message{ex.what()};
        throw libdnf5::cli::CommandExitError(1, M_("Couldn't connect to D-Bus: {}"), error_message);
    }
    auto proxy = sdbus::createProxy(systemd_destination_name, systemd_object_path);
    if (poweroff) {
        proxy->callMethod("Poweroff").onInterface(systemd_manager_interface);
    } else {
        proxy->callMethod("Reboot").onInterface(systemd_manager_interface);
    }
}

void SystemUpgradeRebootCommand::run() {
    std::filesystem::create_symlink(get_data_dir(), get_magic_symlink());
    reboot(get_context());
}

void SystemUpgradeUpgradeCommand::set_argument_parser() {}

void SystemUpgradeUpgradeCommand::configure() {
    auto & ctx = get_context();

    ctx.set_load_system_repo(true);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    ctx.base.get_config().get_cacheonly_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, "all");
    ctx.base.get_config().get_assumeno_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, false);
    ctx.base.get_config().get_assumeyes_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, true);
    ctx.base.get_config().get_clean_requirements_on_remove_option().set(
        libdnf5::Option::Priority::PLUGINDEFAULT, false);
    ctx.base.get_config().get_install_weak_deps_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, false);
}

void SystemUpgradeUpgradeCommand::run() {
    auto & ctx = get_context();

    const auto & transaction_json_path = std::filesystem::path(get_data_dir()) / "system-upgrade-transaction.json";
    auto transaction_json_file = libdnf5::utils::fs::File(transaction_json_path, "r");

    const auto & json = transaction_json_file.read();
    transaction_json_file.close();

    const auto & goal = std::make_unique<libdnf5::Goal>(ctx.base);

    auto settings = libdnf5::GoalJobSettings();
    goal->add_serialized_transaction(json, settings);

    auto transaction = goal->resolve();
    if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw libdnf5::cli::GoalResolveError(transaction);
    }

    transaction.run();
}

}  // namespace dnf5
