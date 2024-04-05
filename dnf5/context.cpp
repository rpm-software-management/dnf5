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

#include "dnf5/context.hpp"

#include "dnf5/offline.hpp"
#include "download_callbacks.hpp"
#include "plugins.hpp"
#include "utils/string.hpp"
#include "utils/url.hpp"

#include <fmt/format.h>
#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf5-cli/tty.hpp>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <libdnf5/rpm/rpm_signature.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>
#include <libdnf5/utils/patterns.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace fs = std::filesystem;

namespace dnf5 {

namespace {

// The `KeyImportRepoCB` class implements callback only for importing repository key.
class KeyImportRepoCB : public libdnf5::repo::RepoCallbacks {
public:
    explicit KeyImportRepoCB(libdnf5::ConfigMain & config) : config(&config) {}

    bool repokey_import(const libdnf5::rpm::KeyInfo & key_info) override {
        // TODO(jrohel): In case `assumeno`==true, the key is not imported. Is it OK to skip import attempt information message?
        //               And what about `assumeyes`==true in silent mode? Print key import message or not?
        if (config->get_assumeno_option().get_value()) {
            return false;
        }

        std::cout << "Importing PGP key 0x" << key_info.get_short_key_id() << ":\n";
        for (auto & user_id : key_info.get_user_ids()) {
            std::cout << " Userid     : \"" << user_id << "\"\n";
        }
        std::cout << " Fingerprint: " << key_info.get_fingerprint() << "\n";
        std::cout << " From       : " << key_info.get_url() << std::endl;

        return libdnf5::cli::utils::userconfirm::userconfirm(*config);
    }

    void repokey_imported([[maybe_unused]] const libdnf5::rpm::KeyInfo & key_info) override {
        std::cout << _("The key was successfully imported.") << std::endl;
    }

private:
    libdnf5::ConfigMain * config;
};

}  // namespace

Context::Context(std::vector<std::unique_ptr<libdnf5::Logger>> && loggers)
    : base(std::move(loggers)),
      plugins(std::make_unique<Plugins>(*this)) {}

Context::~Context() {
    // "Session", which is the parent of "Context", owns objects from dnf5 plugins (command arguments).
    // Objects from plugins must be destroyed before the plugins can be released,
    // otherwise they will reference the released code.
    // TODO(jrohel): Calling clear() is not nice here. Better workflow.
    clear();
}

// TODO(jrohel): Move logic into libdnf?
void Context::apply_repository_setopts() {
    std::vector<std::string> missing_repo_ids;
    for (const auto & setopt : setopts) {
        auto last_dot_pos = setopt.first.rfind('.');
        auto repo_pattern = setopt.first.substr(0, last_dot_pos);
        libdnf5::repo::RepoQuery query(base);
        query.filter_id(repo_pattern, libdnf5::sack::QueryCmp::GLOB);
        query.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);
        if (query.empty()) {
            missing_repo_ids.push_back(repo_pattern);
        }
        auto key = setopt.first.substr(last_dot_pos + 1);
        for (auto & repo : query) {
            try {
                repo->get_config().opt_binds().at(key).new_string(
                    libdnf5::Option::Priority::COMMANDLINE, setopt.second);
            } catch (const std::exception & ex) {
                std::cout << "setopt: \"" + setopt.first + "." + setopt.second + "\": " + ex.what() << std::endl;
            }
        }
    }

    if (!missing_repo_ids.empty()) {
        throw libdnf5::cli::ArgumentParserError(
            M_("No matching repositories for \"{}\""), libdnf5::utils::string::join(missing_repo_ids, ", "));
    }
}


void Context::print_info(std::string_view msg) const {
    if (!quiet) {
        output_stream.get() << msg << std::endl;
    }
}


void Context::update_repo_metadata_from_specs(const std::vector<std::string> & pkg_specs) {
    for (auto & spec : pkg_specs) {
        if (libdnf5::utils::is_file_pattern(spec)) {
            base.get_config().get_optional_metadata_types_option().add_item(
                libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_FILELISTS);
            return;
        }
        if (spec.starts_with('@')) {
            base.get_config().get_optional_metadata_types_option().add_item(
                libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_COMPS);
            return;
        }
    }
}

void Context::update_repo_metadata_from_advisory_options(
    const std::vector<std::string> & names,
    bool security,
    bool bugfix,
    bool enhancement,
    bool newpackage,
    const std::vector<std::string> & severity,
    const std::vector<std::string> & bzs,
    const std::vector<std::string> & cves) {
    bool updateinfo_needed = !names.empty() || security || bugfix || enhancement || newpackage || !severity.empty() ||
                             !bzs.empty() || !cves.empty();
    if (updateinfo_needed) {
        base.get_config().get_optional_metadata_types_option().add_item(
            libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_UPDATEINFO);
    }
}

void Context::load_repos(bool load_system) {
    libdnf5::repo::RepoQuery repos(base);
    repos.filter_enabled(true);
    repos.filter_type(libdnf5::repo::Repo::Type::SYSTEM, libdnf5::sack::QueryCmp::NEQ);

    for (auto & repo : repos) {
        repo->set_callbacks(std::make_unique<dnf5::KeyImportRepoCB>(base.get_config()));
    }

    print_info("Updating and loading repositories:");
    if (load_system) {
        base.get_repo_sack()->load_repos();
    } else {
        base.get_repo_sack()->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
    }

    if (auto download_callbacks = dynamic_cast<DownloadCallbacks *>(base.get_download_callbacks())) {
        download_callbacks->reset_progress_bar();
    }
    print_info("Repositories loaded.");
}

RpmTransCB::RpmTransCB(Context & context) : context(context) {
    multi_progress_bar.set_total_bar_visible_limit(libdnf5::cli::progressbar::MultiProgressBar::NEVER_VISIBLE_LIMIT);
}

RpmTransCB::~RpmTransCB() {
    if (active_progress_bar && active_progress_bar->get_state() != libdnf5::cli::progressbar::ProgressBarState::ERROR) {
        active_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    }
    if (active_progress_bar) {
        multi_progress_bar.print();
    }
}

libdnf5::cli::progressbar::MultiProgressBar * RpmTransCB::get_multi_progress_bar() {
    return &multi_progress_bar;
}

void RpmTransCB::install_progress(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem & item, uint64_t amount, [[maybe_unused]] uint64_t total) {
    active_progress_bar->set_ticks(static_cast<int64_t>(amount));
    if (is_time_to_print()) {
        multi_progress_bar.print();
    }
}

void RpmTransCB::install_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) {
    const char * msg{nullptr};
    switch (item.get_action()) {
        case libdnf5::transaction::TransactionItemAction::UPGRADE:
            msg = "Upgrading ";
            break;
        case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
            msg = "Downgrading ";
            break;
        case libdnf5::transaction::TransactionItemAction::REINSTALL:
            msg = "Reinstalling ";
            break;
        case libdnf5::transaction::TransactionItemAction::INSTALL:
        case libdnf5::transaction::TransactionItemAction::REMOVE:
        case libdnf5::transaction::TransactionItemAction::REPLACED:
            break;
        case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
        case libdnf5::transaction::TransactionItemAction::ENABLE:
        case libdnf5::transaction::TransactionItemAction::DISABLE:
        case libdnf5::transaction::TransactionItemAction::RESET:
        case libdnf5::transaction::TransactionItemAction::SWITCH:
            auto & logger = *context.base.get_logger();
            logger.warning(
                "Unexpected action in TransactionPackage: {}",
                static_cast<std::underlying_type_t<libdnf5::base::Transaction::TransactionRunResult>>(
                    item.get_action()));
            return;
    }
    if (!msg) {
        msg = "Installing ";
    }
    new_progress_bar(static_cast<int64_t>(total), msg + item.get_package().get_full_nevra());
}

void RpmTransCB::install_stop(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
    [[maybe_unused]] uint64_t amount,
    [[maybe_unused]] uint64_t total) {
    multi_progress_bar.print();
}

void RpmTransCB::transaction_progress(uint64_t amount, [[maybe_unused]] uint64_t total) {
    active_progress_bar->set_ticks(static_cast<int64_t>(amount));
    if (is_time_to_print()) {
        multi_progress_bar.print();
    }
}

void RpmTransCB::transaction_start(uint64_t total) {
    new_progress_bar(static_cast<int64_t>(total), "Prepare transaction");
}

void RpmTransCB::transaction_stop([[maybe_unused]] uint64_t total) {
    active_progress_bar->set_ticks(static_cast<int64_t>(total));
    multi_progress_bar.print();
}

void RpmTransCB::uninstall_progress(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem & item, uint64_t amount, [[maybe_unused]] uint64_t total) {
    active_progress_bar->set_ticks(static_cast<int64_t>(amount));
    if (is_time_to_print()) {
        multi_progress_bar.print();
    }
}

void RpmTransCB::uninstall_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) {
    const char * msg{nullptr};
    if (item.get_action() == libdnf5::transaction::TransactionItemAction::REMOVE ||
        item.get_action() == libdnf5::transaction::TransactionItemAction::REPLACED) {
        msg = "Erasing ";
    }
    if (!msg) {
        msg = "Cleanup ";
    }
    new_progress_bar(static_cast<int64_t>(total), msg + item.get_package().get_full_nevra());
}

void RpmTransCB::uninstall_stop(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
    [[maybe_unused]] uint64_t amount,
    [[maybe_unused]] uint64_t total) {
    multi_progress_bar.print();
}


void RpmTransCB::unpack_error(const libdnf5::rpm::TransactionItem & item) {
    active_progress_bar->add_message(
        libdnf5::cli::progressbar::MessageType::ERROR, "Unpack error: " + item.get_package().get_full_nevra());
    active_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::ERROR);
    multi_progress_bar.print();
}

void RpmTransCB::cpio_error(const libdnf5::rpm::TransactionItem & item) {
    active_progress_bar->add_message(
        libdnf5::cli::progressbar::MessageType::ERROR, "Cpio error: " + item.get_package().get_full_nevra());
    active_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::ERROR);
    multi_progress_bar.print();
}

void RpmTransCB::script_error(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type,
    uint64_t return_code) {
    active_progress_bar->add_message(
        libdnf5::cli::progressbar::MessageType::ERROR,
        fmt::format(
            "Error in {} scriptlet: {} return code {}",
            script_type_to_string(type),
            to_full_nevra_string(nevra),
            return_code));
    multi_progress_bar.print();
}

void RpmTransCB::script_start(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type) {
    active_progress_bar->add_message(
        libdnf5::cli::progressbar::MessageType::INFO,
        fmt::format("Running {} scriptlet: {}", script_type_to_string(type), to_full_nevra_string(nevra)));
    multi_progress_bar.print();
}

void RpmTransCB::script_stop(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type,
    [[maybe_unused]] uint64_t return_code) {
    active_progress_bar->add_message(
        libdnf5::cli::progressbar::MessageType::INFO,
        fmt::format("Stop {} scriptlet: {}", script_type_to_string(type), to_full_nevra_string(nevra)));
    multi_progress_bar.print();
}

void RpmTransCB::elem_progress(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem & item,
    [[maybe_unused]] uint64_t amount,
    [[maybe_unused]] uint64_t total) {
    //std::cout << "Element progress: " << header.get_full_nevra() << " " << amount << '/' << total << std::endl;
}

void RpmTransCB::verify_progress(uint64_t amount, [[maybe_unused]] uint64_t total) {
    active_progress_bar->set_ticks(static_cast<int64_t>(amount));
    if (is_time_to_print()) {
        multi_progress_bar.print();
    }
}

void RpmTransCB::verify_start([[maybe_unused]] uint64_t total) {
    new_progress_bar(static_cast<int64_t>(total), "Verify package files");
}

void RpmTransCB::verify_stop([[maybe_unused]] uint64_t total) {
    active_progress_bar->set_ticks(static_cast<int64_t>(total));
    multi_progress_bar.print();
}

void RpmTransCB::new_progress_bar(int64_t total, const std::string & descr) {
    if (active_progress_bar && active_progress_bar->get_state() != libdnf5::cli::progressbar::ProgressBarState::ERROR) {
        active_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    }
    auto progress_bar =
        std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(static_cast<int64_t>(total), descr);
    progress_bar->set_auto_finish(false);
    progress_bar->start();
    active_progress_bar = progress_bar.get();
    multi_progress_bar.add_bar(std::move(progress_bar));
}

bool RpmTransCB::is_time_to_print() {
    auto now = std::chrono::steady_clock::now();
    auto delta = now - prev_print_time;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
    if (ms > 100) {
        // 100ms equals to 10 FPS and that seems to be smooth enough
        prev_print_time = now;
        return true;
    }
    return false;
}

std::chrono::time_point<std::chrono::steady_clock> RpmTransCB::prev_print_time = std::chrono::steady_clock::now();

void Context::download_and_run(libdnf5::base::Transaction & transaction) {
    if (should_store_offline) {
        base.get_config().get_tsflags_option().set(libdnf5::Option::Priority::RUNTIME, "test");
    }
    transaction.download();

    if (base.get_config().get_downloadonly_option().get_value()) {
        return;
    }

    print_info("\nRunning transaction");

    // Compute the total number of transaction actions (number of bars)
    // Total number of actions = number of packages in the transaction +
    //                           action of verifying package files if new package files are present in the transaction +
    //                           action of preparing transaction
    const auto & trans_packages = transaction.get_transaction_packages();
    auto num_of_actions = trans_packages.size() + 1;
    for (auto & trans_pkg : trans_packages) {
        if (libdnf5::transaction::transaction_item_action_is_inbound(trans_pkg.get_action())) {
            ++num_of_actions;
            break;
        }
    }

    auto callbacks = std::make_unique<RpmTransCB>(*this);
    callbacks->get_multi_progress_bar()->set_total_num_of_bars(num_of_actions);
    transaction.set_callbacks(std::move(callbacks));

    transaction.set_description(get_cmdline());

    if (comment) {
        transaction.set_comment(comment);
    }

    auto result = transaction.run();
    print_info("");
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
    // TODO(mblaha): print a summary of successful transaction

    if (should_store_offline) {
        store_offline(transaction);
        std::cout << "Transaction stored to be performed offline. Run `dnf5 offline reboot` to reboot and run the "
                     "transaction. To cancel the transaction and delete the downloaded files, use `dnf5 "
                     "offline clean`."
                  << std::endl;
    }
}

void Context::store_offline(libdnf5::base::Transaction & transaction) {
    const auto & installroot = base.get_config().get_installroot_option().get_value();
    const auto & offline_datadir = installroot / dnf5::offline::DEFAULT_DATADIR.relative_path();
    std::filesystem::create_directories(offline_datadir);

    const std::filesystem::path state_path{offline_datadir / dnf5::offline::TRANSACTION_STATE_FILENAME};
    dnf5::offline::OfflineTransactionState state{state_path};

    if (state.get_data().status != dnf5::offline::STATUS_DOWNLOAD_INCOMPLETE) {
        std::cout << "There is already an offline transaction queued, initiated by the following command:" << std::endl
                  << "\t" << state.get_data().cmd_line << std::endl
                  << "Continuing will cancel the old offline transaction and replace it with this one." << std::endl;
        if (!libdnf5::cli::utils::userconfirm::userconfirm(base.get_config())) {
            throw libdnf5::cli::SilentCommandExitError(0);
        }
    }

    const std::filesystem::path transaction_json_path{offline_datadir / dnf5::offline::TRANSACTION_JSON_FILENAME};

    const std::string json = transaction.serialize();

    auto transaction_json_file = libdnf5::utils::fs::File(transaction_json_path, "w");

    transaction_json_file.write(json);
    transaction_json_file.close();

    state.get_data().status = dnf5::offline::STATUS_DOWNLOAD_COMPLETE;
    state.get_data().cachedir = base.get_config().get_cachedir_option().get_value();

    std::vector<std::string> command_vector;
    auto * current_command = get_selected_command();
    while (current_command != get_root_command()) {
        command_vector.insert(command_vector.begin(), current_command->get_argument_parser_command()->get_id());
        current_command = current_command->get_parent_command();
    }
    state.get_data().verb = libdnf5::utils::string::join(command_vector, " ");
    state.get_data().cmd_line = get_cmdline();

    const auto & detected_releasever = libdnf5::Vars::detect_release(base.get_weak_ptr(), installroot);
    if (detected_releasever != nullptr) {
        state.get_data().system_releasever = *detected_releasever;
    }
    state.get_data().target_releasever = base.get_vars()->get_value("releasever");

    if (!base.get_config().get_module_platform_id_option().empty()) {
        state.get_data().module_platform_id = base.get_config().get_module_platform_id_option().get_value();
    }

    state.write();
}

libdnf5::Goal * Context::get_goal(bool new_if_not_exist) {
    if (!goal && new_if_not_exist) {
        goal = std::make_unique<libdnf5::Goal>(base);
    }
    return goal.get();
}

void Command::goal_resolved() {
    auto & transaction = *get_context().get_transaction();
    if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw libdnf5::cli::GoalResolveError(transaction);
    }
}

/// Returns file and directory paths that begins with `path_to_complete`.
/// Files must match `regex_pattern`.
static std::pair<std::vector<std::string>, std::vector<std::string>> complete_paths(
    const std::string & path_to_complete, const std::regex & regex_pattern) {
    std::pair<std::vector<std::string>, std::vector<std::string>> ret;

    const fs::path ppath_to_complete(path_to_complete);
    fs::path parent_path = ppath_to_complete.parent_path();
    if (parent_path.empty()) {
        parent_path = ".";
    }

    const bool path_to_complete_prefix_dot_slash = path_to_complete[0] == '.' && path_to_complete[1] == '/';
    const bool filename_to_complete_starts_with_dot = ppath_to_complete.filename().native()[0] == '.';
    std::error_code ec;  // Do not report errors when constructing a directory iterator
    for (const auto & dir_entry : fs::directory_iterator(parent_path, ec)) {
        const auto filename = dir_entry.path().filename();

        // Skips hidden entries (starting with a dot) unless explicitly requested by `path_to_complete`.
        if (!filename_to_complete_starts_with_dot && filename.native()[0] == '.') {
            continue;
        }

        std::string dir_entry_path;
        const auto & raw_dir_entry_path = dir_entry.path().native();
        if (path_to_complete_prefix_dot_slash) {
            dir_entry_path = raw_dir_entry_path;
        } else {
            dir_entry_path = raw_dir_entry_path[0] == '.' && raw_dir_entry_path[1] == '/'
                                 ? raw_dir_entry_path.substr(2)  // remove "./" prefix
                                 : raw_dir_entry_path;
        }

        if (dir_entry_path.compare(0, path_to_complete.length(), path_to_complete) == 0) {
            // Adds the directory.
            // Only directories that contain files that match the pattern or contain subdirectories are added.
            if (dir_entry.is_directory()) {
                bool complete = false;
                for (const auto & subdir_entry : fs::directory_iterator(dir_entry.path(), ec)) {
                    if ((subdir_entry.is_regular_file() &&
                         std::regex_match(subdir_entry.path().filename().native(), regex_pattern)) ||
                        subdir_entry.is_directory()) {
                        complete = true;
                        break;
                    }
                }
                if (complete) {
                    ret.second.push_back(dir_entry_path + '/');
                }
                continue;
            }

            // Adds the file if it matches the pattern.
            if (dir_entry.is_regular_file() && std::regex_match(filename.native(), regex_pattern)) {
                ret.first.push_back(dir_entry_path);
            }
        }
    }

    return ret;
}

std::vector<std::string> match_specs(
    Context & ctx,
    const std::string & pattern,
    bool installed,
    bool available,
    bool paths,
    bool nevra_for_same_name,
    const char * file_name_regex) {
    auto & base = ctx.base;

    base.get_config().get_assumeno_option().set(libdnf5::Option::Priority::RUNTIME, true);
    ctx.set_quiet(true);

    base.load_config();
    base.setup();

    // optimization - disable the search for matching installed and available packages for file patterns
    if (libdnf5::utils::is_file_pattern(pattern)) {
        installed = available = false;
    }

    if (available) {
        try {
            // create rpm repositories according configuration files
            base.get_repo_sack()->create_repos_from_system_configuration();
            base.get_config().get_optional_metadata_types_option().set(
                libdnf5::Option::Priority::RUNTIME, libdnf5::OptionStringSet::ValueType{});

            ctx.apply_repository_setopts();

            libdnf5::repo::RepoQuery enabled_repos(base);
            enabled_repos.filter_enabled(true);
            enabled_repos.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);
            for (auto & repo : enabled_repos.get_data()) {
                repo->set_sync_strategy(libdnf5::repo::Repo::SyncStrategy::ONLY_CACHE);
                repo->get_config().get_skip_if_unavailable_option().set(libdnf5::Option::Priority::RUNTIME, true);
            }

            ctx.load_repos(installed);
        } catch (...) {
            // Ignores errors when completing available packages, other completions may still work.
        }
    } else if (installed) {
        try {
            base.get_repo_sack()->load_repos(libdnf5::repo::Repo::Type::SYSTEM);
        } catch (...) {
            // Ignores errors when completing installed packages, other completions may still work.
        }
    }

    std::set<std::string> result_set;
    {
        libdnf5::rpm::PackageQuery matched_pkgs_query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_ignore_case(false);
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        matched_pkgs_query.resolve_pkg_spec(pattern + '*', settings, true);

        for (const auto & package : matched_pkgs_query) {
            auto [it, inserted] = result_set.insert(package.get_name());

            // Package name was already present - not inserted. There are multiple packages with the same name.
            // If requested, removes the name and inserts a full nevra for these packages.
            if (nevra_for_same_name && !inserted) {
                result_set.erase(it);
                libdnf5::rpm::PackageQuery name_query(matched_pkgs_query);
                name_query.filter_name({package.get_name()});
                for (const auto & pkg : name_query) {
                    result_set.insert(pkg.get_full_nevra());
                    matched_pkgs_query.remove(pkg);
                }
            }
        }
    }

    std::vector<std::string> file_paths;
    std::vector<std::string> dir_paths;
    if (paths) {
        if (!file_name_regex) {
            file_name_regex = ".*";
        }
        std::regex regex_pattern(file_name_regex, std::regex_constants::nosubs | std::regex_constants::optimize);
        std::tie(file_paths, dir_paths) = complete_paths(pattern, regex_pattern);
        std::sort(file_paths.begin(), file_paths.end());
        std::sort(dir_paths.begin(), dir_paths.end());
    }

    std::vector<std::string> result;
    result.reserve(file_paths.size() + dir_paths.size() + result_set.size());
    std::move(file_paths.begin(), file_paths.end(), std::back_inserter(result));
    std::move(result_set.begin(), result_set.end(), std::back_inserter(result));
    std::move(dir_paths.begin(), dir_paths.end(), std::back_inserter(result));
    return result;
}

}  // namespace dnf5
