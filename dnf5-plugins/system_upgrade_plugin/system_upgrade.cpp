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

#include "utils/string.hpp"

#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_path.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>
#include <sys/wait.h>
#include <systemd/sd-journal.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

using namespace libdnf5::cli;

const std::string & ID_TO_IDENTIFY_BOOTS = OFFLINE_STARTED_ID;

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

int call(const std::string & command, const std::vector<std::string> & args) {
    std::vector<char *> c_args;
    c_args.emplace_back(const_cast<char *>(command.c_str()));
    for (const auto & arg : args) {
        c_args.emplace_back(const_cast<char *>(arg.c_str()));
    }
    c_args.emplace_back(nullptr);

    const auto pid = fork();
    if (pid == -1) {
        return -1;
    }
    if (pid == 0) {
        int rc = execvp(command.c_str(), c_args.data());
        exit(rc == 0 ? 0 : -1);
    } else {
        int status;
        int rc = waitpid(pid, &status, 0);
        if (rc == -1) {
            return -1;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return -1;
    }
}

namespace dnf5 {

/// Helper for displaying messages with Plymouth
///
/// Derived from DNF 4 system-upgrade PlymouthOutput implementation. Filters
/// duplicate calls, and stops calling the plymouth binary if we fail to
/// contact it.
class PlymouthOutput {
public:
    bool ping() { return plymouth({"ping"}); }
    bool set_mode() { return plymouth({"change-mode", "--system-upgrade"}); }
    bool message(const std::string & message) {
        if (last_message.has_value() && message == last_message) {
            plymouth({"hide-message", "--text", last_message.value()});
        }
        last_message = message;
        return plymouth({"display-message", "--text", message});
    }
    bool progress(const int percentage) {
        return plymouth({"system-update", "--progress", std::to_string(percentage)});
    }

private:
    bool alive = true;
    std::map<std::string, std::vector<std::string>> last_subcommand_args;
    std::optional<std::string> last_message;
    bool plymouth(const std::vector<std::string> & args) {
        const auto & command = args.at(0);
        const auto & last_args = last_subcommand_args.find(command);

        bool dupe_cmd = (last_args != last_subcommand_args.end() && args == last_args->second);
        if ((alive && !dupe_cmd) || command == "--ping") {
            alive = call(PATH_TO_PLYMOUTH, args) == 0;
            last_subcommand_args[command] = args;
        }
        return alive;
    }
};

/// Extend RpmTransCB to also display messages with Plymouth
class PlymouthTransCB : public RpmTransCB {
public:
    PlymouthTransCB(PlymouthOutput plymouth) : plymouth(std::move(plymouth)) {}
    void elem_progress(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        RpmTransCB::elem_progress(item, amount, total);

        plymouth.progress(static_cast<int>(100 * static_cast<double>(amount) / static_cast<double>(total)));

        std::string action;
        switch (item.get_action()) {
            case libdnf5::transaction::TransactionItemAction::UPGRADE:
                action = "Upgrading";
                break;
            case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
                action = "Downgrading";
                break;
            case libdnf5::transaction::TransactionItemAction::REINSTALL:
                action = "Reinstalling";
                break;
            case libdnf5::transaction::TransactionItemAction::INSTALL:
                action = "Installing";
                break;
            case libdnf5::transaction::TransactionItemAction::REMOVE:
                action = "Removing";
                break;
            case libdnf5::transaction::TransactionItemAction::REPLACED:
                action = "Replacing";
                break;
            case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
            case libdnf5::transaction::TransactionItemAction::ENABLE:
            case libdnf5::transaction::TransactionItemAction::DISABLE:
            case libdnf5::transaction::TransactionItemAction::RESET:
                throw std::logic_error(fmt::format(
                    "Unexpected action in TransactionPackage: {}",
                    static_cast<std::underlying_type_t<libdnf5::base::Transaction::TransactionRunResult>>(
                        item.get_action())));
                break;
        }
        const auto & message = fmt::format("[{}/{}] {} {}...", amount, total, action, item.get_package().get_name());
        plymouth.message(message);
    }

private:
    PlymouthOutput plymouth;
};

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
    register_subcommand(std::make_unique<SystemUpgradeCleanCommand>(get_context()));
    register_subcommand(std::make_unique<SystemUpgradeDownloadCommand>(get_context()));
    register_subcommand(std::make_unique<SystemUpgradeRebootCommand>(get_context()));
    register_subcommand(std::make_unique<SystemUpgradeUpgradeCommand>(get_context()));
    register_subcommand(std::make_unique<SystemUpgradeLogCommand>(get_context()));
}

SystemUpgradeSubcommand::SystemUpgradeSubcommand(Context & context, const std::string & name)
    : Command(context, name),
      datadir(std::filesystem::path{libdnf5::SYSTEM_STATE_DIR} / "system-upgrade"),
      state(datadir / "state.toml") {}

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

    system_releasever = *libdnf5::Vars::detect_release(ctx.base.get_weak_ptr(), installroot);
    target_releasever = ctx.base.get_vars()->get_value("releasever");
}

void SystemUpgradeSubcommand::log_status(const std::string & message, const std::string & message_id) const {
    const auto & version = get_application_version();
    const std::string & version_string = fmt::format("{}.{}.{}", version.major, version.minor, version.micro);
    sd_journal_send(
        "MESSAGE=%s",
        message.c_str(),
        "MESSAGE_ID=%s",
        message_id.c_str(),
        "SYSTEM_RELEASEVER=%s",
        get_system_releasever().c_str(),
        "TARGET_RELEASEVER=%s",
        get_target_releasever().c_str(),
        "DNF_VERSION=%s",
        version_string.c_str(),
        NULL);
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

    // Check --releasever
    if (get_target_releasever() == get_system_releasever()) {
        throw libdnf5::cli::CommandExitError(1, M_("Need a --releasever greater than the current system version."));
    }

    ctx.set_load_system_repo(true);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    ctx.base.get_config().get_cachedir_option().set(
        libdnf5::Option::Priority::PLUGINDEFAULT, get_cachedir()->get_value());
}

void SystemUpgradeDownloadCommand::run() {
    auto & ctx = get_context();

    const auto & goal = std::make_unique<libdnf5::Goal>(ctx.base);

    auto & conf = ctx.base.get_config();
    conf.get_tsflags_option().set(libdnf5::Option::Priority::RUNTIME, "test");

    if (no_downgrade->get_value()) {
        goal->add_rpm_upgrade();
    } else {
        goal->add_rpm_distro_sync();
    }

    auto transaction = goal->resolve();
    if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw libdnf5::cli::GoalResolveError(transaction);
    }

    ctx.download_and_run(transaction);

    if (transaction.get_transaction_packages_count() == 0) {
        throw libdnf5::cli::CommandExitError(
            1, M_("The system-upgrade transaction is empty; your system is already up-to-date."));
    }

    const std::string json = transaction.serialize();
    const auto & transaction_json_path = std::filesystem::path(get_datadir()) / "system-upgrade-transaction.json";
    auto transaction_json_file = libdnf5::utils::fs::File(transaction_json_path, "w");

    transaction_json_file.write(json);
    transaction_json_file.close();

    auto state = get_state();
    state.get_data().cachedir = get_cachedir()->get_value();
    state.get_data().system_releasever = get_system_releasever();
    state.get_data().target_releasever = get_target_releasever();
    state.get_data().status = STATUS_DOWNLOAD_COMPLETE;
    libdnf5::repo::RepoQuery enabled_repos(ctx.base);
    enabled_repos.filter_enabled(true);
    enabled_repos.filter_type(libdnf5::repo::Repo::Type::SYSTEM, libdnf5::sack::QueryCmp::NEQ);
    for (const auto & repo : enabled_repos) {
        state.get_data().enabled_repos.emplace_back(repo->get_id());
    }
    libdnf5::repo::RepoQuery disabled_repos(ctx.base);
    disabled_repos.filter_enabled(false);
    disabled_repos.filter_type(libdnf5::repo::Repo::Type::SYSTEM, libdnf5::sack::QueryCmp::NEQ);
    for (const auto & repo : disabled_repos) {
        state.get_data().disabled_repos.emplace_back(repo->get_id());
    }
    state.write();

    std::cout << "Download complete! Use `dnf5 system-upgrade reboot` to start the upgrade." << std::endl
              << "To cancel the upgrade and delete the downloaded upgrade files, use `dnf5 system-upgrade clean`."
              << std::endl;
}

void check_state(const OfflineTransactionState & state) {
    const auto & read_exception = state.get_read_exception();
    if (read_exception != nullptr) {
        try {
            std::rethrow_exception(read_exception);
        } catch (const std::exception & ex) {
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

void clean(Context & ctx, const std::filesystem::path & datadir) {
    ctx.base.get_logger()->info("Cleaning up downloaded data...");

    for (const auto & entry : std::filesystem::directory_iterator(datadir)) {
        std::filesystem::remove_all(entry.path());
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
    auto & ctx = get_context();

    auto state = get_state();
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

    std::cout << "The system will now reboot to upgrade to release version " << state.get_data().target_releasever
              << ". Do you want to continue?" << std::endl;
    if (!libdnf5::cli::utils::userconfirm::userconfirm(ctx.base.get_config())) {
        return;
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

    auto state = get_state();
    check_state(state);

    // Set same set of enabled/disabled repos used during `system-upgrade download`
    for (const auto & repo_id : state.get_data().enabled_repos) {
        ctx.setopts.emplace_back(repo_id + ".enabled", "1");
    }
    for (const auto & repo_id : state.get_data().disabled_repos) {
        ctx.setopts.emplace_back(repo_id + ".disabled", "1");
    }

    // Don't try to refresh metadata, we are offline
    ctx.base.get_config().get_cacheonly_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, "all");
    // Don't ask any questions
    ctx.base.get_config().get_assumeyes_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, true);
    // Override `assumeno` too since it takes priority over `assumeyes`
    ctx.base.get_config().get_assumeno_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, false);
    // Upgrade operation already removes all element that must be removed.
    // Additional removal could trigger unwanted changes in transaction.
    ctx.base.get_config().get_clean_requirements_on_remove_option().set(
        libdnf5::Option::Priority::PLUGINDEFAULT, false);
    ctx.base.get_config().get_install_weak_deps_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, false);
}

void SystemUpgradeUpgradeCommand::run() {
    auto & ctx = get_context();

    log_status("Starting offline transaction. This will take a while.", OFFLINE_STARTED_ID);

    if (!std::filesystem::is_symlink(get_magic_symlink())) {
        throw libdnf5::cli::CommandExitError(0, M_("Trigger file does not exist. Exiting."));
    }

    const auto & symlinked_path = std::filesystem::read_symlink(get_magic_symlink());
    if (symlinked_path != get_datadir()) {
        throw libdnf5::cli::CommandExitError(0, M_("Another upgrade tool is running. Exiting."));
    }

    std::filesystem::remove(get_magic_symlink());

    auto state = get_state();
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

    PlymouthOutput plymouth;
    auto callbacks = std::make_unique<PlymouthTransCB>(plymouth);
    /* callbacks->get_multi_progress_bar()->set_total_num_of_bars(num_of_actions); */
    transaction.set_callbacks(std::move(callbacks));

    const auto result = transaction.run();
    std::cout << std::endl;
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

    for (auto const & entry : transaction.get_gpg_signature_problems()) {
        std::cerr << entry << std::endl;
    }

    std::string upgrade_complete_message;
    if (state.get_data().poweroff_after) {
        upgrade_complete_message = "Upgrade complete! Cleaning up and powering off...";
    } else {
        upgrade_complete_message = "Upgrade complete! Cleaning up and rebooting...";
    }
    plymouth.message(upgrade_complete_message);
    log_status(upgrade_complete_message, OFFLINE_STARTED_ID);

    // If the upgrade succeeded, remove downloaded data
    clean(ctx, get_datadir());

    reboot(ctx, state.get_data().poweroff_after);
}

void SystemUpgradeCleanCommand::set_argument_parser() {
    SystemUpgradeSubcommand::set_argument_parser();
}

void SystemUpgradeCleanCommand::run() {
    auto & ctx = get_context();
    clean(ctx, get_datadir());
}

struct BootEntry {
    std::string boot_id;
    std::string timestamp;
    std::string system_releasever;
    std::string target_releasever;
};

std::string get_journal_field(sd_journal * journal, const std::string & field) {
    const char * data = nullptr;
    size_t length = 0;
    auto rc = sd_journal_get_data(journal, field.c_str(), reinterpret_cast<const void **>(&data), &length);
    if (rc < 0 || data == nullptr) {
        return "";
    }
    const auto prefix_length = field.length() + 1;
    return std::string{data + prefix_length, length - prefix_length};
}

std::vector<BootEntry> find_boots(const std::string & message_id) {
    std::vector<BootEntry> boots{};

    sd_journal * journal = nullptr;
    auto rc = sd_journal_open(&journal, SD_JOURNAL_LOCAL_ONLY);
    if (rc < 0) {
        throw libdnf5::RuntimeError(M_("Error reading journal: {}"), std::string{std::strerror(-rc)});
    }

    const auto & uid_filter_string = fmt::format("MESSAGE_ID={}", message_id);
    rc = sd_journal_add_match(journal, uid_filter_string.c_str(), 0);
    if (rc < 0) {
        throw libdnf5::RuntimeError(M_("Add journal match failed: {}"), std::string{std::strerror(-rc)});
    }
    SD_JOURNAL_FOREACH(journal) {
        uint64_t usec = 0;
        rc = sd_journal_get_realtime_usec(journal, &usec);
        auto sec = usec / (1000 * 1000);
        boots.emplace_back(BootEntry{
            get_journal_field(journal, "_BOOT_ID"),
            libdnf5::utils::string::format_epoch(sec),
            get_journal_field(journal, "SYSTEM_RELEASEVER"),
            get_journal_field(journal, "TARGET_RELEASEVER"),
        });
    }

    return boots;
}

void list_logs() {
    const auto & boot_entries = find_boots(ID_TO_IDENTIFY_BOOTS);

    std::cout << "The following boots appear to contain upgrade logs:" << std::endl;
    for (size_t index = 0; index < boot_entries.size(); index += 1) {
        const auto & entry = boot_entries[index];
        std::cout << fmt::format(
                         "{} / {}: {} {}→{}",
                         index + 1,
                         entry.boot_id,
                         entry.timestamp,
                         entry.system_releasever,
                         entry.target_releasever)
                  << std::endl;
    }
    if (boot_entries.empty()) {
        std::cout << "No logs were found." << std::endl;
    }
}

void show_log(size_t boot_index) {
    const auto & boot_entries = find_boots(ID_TO_IDENTIFY_BOOTS);
    if (boot_index >= boot_entries.size()) {
        throw libdnf5::cli::CommandExitError(1, M_("Cannot find logs with this index."));
    }

    const auto & boot_id = boot_entries[boot_index].boot_id;
    const auto rc = call(PATH_TO_JOURNALCTL, {"--boot", boot_id});

    if (rc != 0 && rc != 141) {
        throw libdnf5::cli::CommandExitError(1, M_("Unable to match systemd journal entry."));
    }
}

void SystemUpgradeLogCommand::set_argument_parser() {
    SystemUpgradeSubcommand::set_argument_parser();

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("Show logs from past upgrades");

    number = dynamic_cast<libdnf5::OptionString *>(parser.add_init_value(std::make_unique<libdnf5::OptionString>("")));

    auto * number_arg = parser.add_new_named_arg("number");
    number_arg->set_long_name("number");
    number_arg->set_has_value(true);

    number_arg->set_description("Which log to show. Run without any arguments to get a list of available logs.");
    number_arg->link_value(number);
    cmd.register_named_arg(number_arg);
}

void SystemUpgradeLogCommand::run() {
    if (number->get_value().empty()) {
        list_logs();
    } else {
        std::string number_string{number->get_value()};
        show_log(std::stoul(number_string) - 1);
    }
}

}  // namespace dnf5
