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

#include <exception>
#include <iostream>

OfflineTransactionState::OfflineTransactionState(std::filesystem::path path) : path(std::move(path)) {
    read();
}
void OfflineTransactionState::read() {
    try {
        const auto & value = toml::parse(path);
        data = toml::find<OfflineTransactionStateData>(value, STATE_HEADER);
        if (data.state_version != STATE_VERSION) {
            throw libdnf5::RuntimeError(M_("incompatible version of state data"));
        }
    } catch (const std::exception & ex) {
        read_exception = std::current_exception();
        data = OfflineTransactionStateData{};
    }
}
void OfflineTransactionState::write() {
    auto file = libdnf5::utils::fs::File(path, "w");
    file.write(toml::format(toml::value{{STATE_HEADER, data}}));
    file.close();
}

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
    register_subcommand(std::make_unique<SystemUpgradeRebootCommand>(get_context()));
    register_subcommand(std::make_unique<SystemUpgradeUpgradeCommand>(get_context()));
}

SystemUpgradeSubcommand::SystemUpgradeSubcommand(Context & context, const std::string & name) : Command(context, name) {
    datadir = std::filesystem::path{libdnf5::SYSTEM_STATE_DIR} / "system-upgrade";
}

void SystemUpgradeSubcommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cachedir =
        dynamic_cast<libdnf5::OptionPath *>(parser.add_init_value(std::make_unique<libdnf5::OptionPath>(datadir)));

    auto * download_dir_arg = parser.add_new_named_arg("downloaddir");
    download_dir_arg->set_long_name("downloaddir");
    download_dir_arg->set_description("Redirect download of packages to provided <path>");
    download_dir_arg->link_value(cachedir);
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
    system_releasever = *libdnf5::Vars::detect_release(ctx.base.get_weak_ptr(), installroot);
    target_releasever = ctx.base.get_vars()->get_value("releasever");

    // TODO: uncomment this. Easier to test with it commented out.
    /* if (target_releasever == system_releasever) { */
    /*     throw libdnf5::cli::CommandExitError(1, M_("Need a --releasever greater than the current system version.")); */
    /* } */

    ctx.set_load_system_repo(true);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    ctx.base.get_config().get_cachedir_option().set(
        libdnf5::Option::Priority::PLUGINDEFAULT, get_cachedir()->get_value());
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

    const auto & transaction_json_path = std::filesystem::path(get_datadir()) / "system-upgrade-transaction.json";
    auto transaction_json_file = libdnf5::utils::fs::File(transaction_json_path, "w");

    transaction_json_file.write(json);
    transaction_json_file.close();

    auto state = read_or_make_state();
    state.get_data().cachedir = get_cachedir()->get_value();
    state.get_data().system_releasever = system_releasever;
    state.get_data().target_releasever = target_releasever;
    state.get_data().status = STATUS_DOWNLOAD_COMPLETE;
    state.write();
}

void check_state(const OfflineTransactionState & state) {
    const auto & read_exception = state.get_read_exception();
    if (read_exception != nullptr) {
        try {
            std::rethrow_exception(read_exception);
        } catch (const std::exception & ex) {
            std::cout << "ex.what rethrown is " << ex.what() << std::endl;
            const std::string message{ex.what()};
            throw libdnf5::cli::CommandExitError(
                1, M_("Error reading state: {}. Rerun `dnf5 system-upgrade download [OPTIONS]`."), message);
        }
    }
}

void reboot(Context & ctx, bool poweroff = false) {
    const std::string systemd_destination_name{"org.freedesktop.systemd1"};
    const std::string systemd_object_path{"/org/freedesktop/systemd1"};
    const std::string systemd_manager_interface{"org.freedesktop.systemd1.Manager"};

    if (std::getenv("DNF_SYSTEM_UPGRADE_NO_REBOOT")) {
        ctx.base.get_logger()->info("Reboot turned off, not rebooting.");
        return;
    }

    poweroff = !poweroff;
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

void SystemUpgradeRebootCommand::set_argument_parser() {
    SystemUpgradeSubcommand::set_argument_parser();

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("Prepare the system to perform the upgrade and reboots to start the upgrade.");

    poweroff_after =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(true)));

    auto * poweroff_after_arg = parser.add_new_named_arg("poweroff");
    poweroff_after_arg->set_long_name("poweroff");
    poweroff_after_arg->set_description("Power off the system after the operation is complete");
    poweroff_after_arg->link_value(poweroff_after);

    cmd.register_named_arg(poweroff_after_arg);
}

void SystemUpgradeRebootCommand::run() {
    auto state = read_or_make_state();
    check_state(state);
    if (state.get_data().status != STATUS_DOWNLOAD_COMPLETE) {
        throw libdnf5::cli::CommandExitError(1, M_("system is not ready for upgrade"));
    }
    if (std::filesystem::is_symlink(get_magic_symlink())) {
        throw libdnf5::cli::CommandExitError(1, M_("upgrade is already scheduled"));
    }

    if (!std::filesystem::is_directory(get_datadir())) {
        throw libdnf5::cli::CommandExitError(1, M_("data directory {} does not exist"), get_datadir().string());
    }

    std::filesystem::create_symlink(get_datadir(), get_magic_symlink());

    state.get_data().status = STATUS_READY;
    state.get_data().poweroff_after = poweroff_after->get_value();
    state.write();

    reboot(get_context(), poweroff_after->get_value());
}

void SystemUpgradeUpgradeCommand::set_argument_parser() {}

void SystemUpgradeUpgradeCommand::configure() {
    SystemUpgradeSubcommand::configure();
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

    if (!std::filesystem::is_symlink(get_magic_symlink())) {
        throw libdnf5::cli::CommandExitError(0, M_("Trigger file does not exist. Exiting."));
    }

    const auto & symlinked_path = std::filesystem::read_symlink(get_magic_symlink());
    if (symlinked_path != get_datadir()) {
        throw libdnf5::cli::CommandExitError(0, M_("Another upgrade tool is running. Exiting."));
    }

    std::filesystem::remove(get_magic_symlink());

    auto state = read_or_make_state();
    check_state(state);
    if (state.get_data().status != STATUS_READY) {
        throw libdnf5::cli::CommandExitError(1, M_("Use `dnf5 system-upgrade reboot` to begin the upgrade."));
    }

    const auto & transaction_json_path = std::filesystem::path(get_datadir()) / "system-upgrade-transaction.json";
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

    const auto result = transaction.run();
    if (result != libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
        std::cerr << "Transaction failed: " << libdnf5::base::Transaction::transaction_result_to_string(result)
                  << std::endl;
        for (auto const & entry : transaction.get_gpg_signature_problems()) {
            std::cerr << entry << std::endl;
        }
        for (auto & problem : transaction.get_transaction_problems()) {
            std::cerr << "  - " << problem << std::endl;
        }
        throw libdnf5::cli::SilentCommandExitError(1);
    }
}

void SystemUpgradeCleanCommand::run() {}

}  // namespace dnf5
