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

#include "download_callbacks.hpp"
#include "plugins.hpp"
#include "utils.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/url.hpp"

#include "libdnf-cli/utils/userconfirm.hpp"

#include <fmt/format.h>
#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf-cli/tty.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/base/goal.hpp>
#include <libdnf/conf/const.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/rpm_signature.hpp>
#include <libdnf/utils/patterns.hpp>

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
class KeyImportRepoCB : public libdnf::repo::RepoCallbacks {
public:
    explicit KeyImportRepoCB(libdnf::ConfigMain & config) : config(&config) {}

    bool repokey_import(
        const std::string & id,
        const std::vector<std::string> & user_ids,
        const std::string & fingerprint,
        const std::string & url,
        [[maybe_unused]] long int timestamp) override {
        // TODO(jrohel): In case `assumeno`==true, the key is not imported. Is it OK to skip import atempt information message?
        //               And what about `assumeyes`==true in silent mode? Print key import message or not?
        if (config->get_assumeno_option().get_value()) {
            return false;
        }

        auto tmp_id = id.size() > 8 ? id.substr(id.size() - 8) : id;
        std::cout << "Importing PGP key 0x" << id << ":\n";
        for (auto & user_id : user_ids) {
            std::cout << " Userid     : \"" << user_id << "\"\n";
        }
        std::cout << " Fingerprint: " << fingerprint << "\n";
        std::cout << " From       : " << url << std::endl;

        return libdnf::cli::utils::userconfirm::userconfirm(*config);
    }

private:
    libdnf::ConfigMain * config;
};

}  // namespace

Context::Context(std::vector<std::unique_ptr<libdnf::Logger>> && loggers)
    : base(std::move(loggers)),
      plugins(std::make_unique<Plugins>(*this)) {}

Context::~Context() {
    // "Session", which is the parent of "Context", owns objects from dnf5 plugins (command arguments).
    // Objects from plugins must be destroyed before the plugins can be released,
    // otherwise they will reference the released code.
    // TODO(jrohel): Calling clear() is not nice here. Better workflow.
    clear();
}

// TODO(jrohel): Move logic into lidnf?
void Context::apply_repository_setopts() {
    for (const auto & setopt : setopts) {
        auto last_dot_pos = setopt.first.rfind('.');
        auto repo_pattern = setopt.first.substr(0, last_dot_pos);
        libdnf::repo::RepoQuery query(base);
        query.filter_id(repo_pattern, libdnf::sack::QueryCmp::GLOB);
        query.filter_type(libdnf::repo::Repo::Type::AVAILABLE);
        auto key = setopt.first.substr(last_dot_pos + 1);
        for (auto & repo : query) {
            try {
                repo->get_config().opt_binds().at(key).new_string(libdnf::Option::Priority::COMMANDLINE, setopt.second);
            } catch (const std::exception & ex) {
                std::cout << "setopt: \"" + setopt.first + "." + setopt.second + "\": " + ex.what() << std::endl;
            }
        }
    }
}


void Context::print_info(std::string_view msg) const {
    if (!quiet) {
        std::cout << msg << std::endl;
    }
}


void Context::update_repo_metadata_from_specs(const std::vector<std::string> & pkg_specs) {
    for (auto & spec : pkg_specs) {
        if (libdnf::utils::is_file_pattern(spec)) {
            base.get_config().get_optional_metadata_types_option().add_item(
                libdnf::Option::Priority::RUNTIME, libdnf::METADATA_TYPE_FILELISTS);
            return;
        }
    }
}


void Context::load_repos(bool load_system) {
    libdnf::repo::RepoQuery repos(base);
    repos.filter_enabled(true);
    repos.filter_type(libdnf::repo::Repo::Type::SYSTEM, libdnf::sack::QueryCmp::NEQ);

    for (auto & repo : repos) {
        repo->set_callbacks(std::make_unique<dnf5::KeyImportRepoCB>(base.get_config()));
    }

    print_info("Updating and loading repositories:");
    base.get_repo_sack()->update_and_load_enabled_repos(load_system);
    if (auto download_callbacks = dynamic_cast<DownloadCallbacks *>(base.get_download_callbacks())) {
        download_callbacks->reset_progress_bar();
    }
    print_info("Repositories loaded.");
}

namespace {

class RpmTransCB : public libdnf::rpm::TransactionCallbacks {
public:
    RpmTransCB() {
        multi_progress_bar.set_total_bar_visible_limit(libdnf::cli::progressbar::MultiProgressBar::NEVER_VISIBLE_LIMIT);
    }

    ~RpmTransCB() {
        if (active_progress_bar &&
            active_progress_bar->get_state() != libdnf::cli::progressbar::ProgressBarState::ERROR) {
            active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        }
        if (active_progress_bar) {
            multi_progress_bar.print();
        }
    }

    libdnf::cli::progressbar::MultiProgressBar * get_multi_progress_bar() { return &multi_progress_bar; }

    void install_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void install_start(const libdnf::rpm::TransactionItem & item, uint64_t total) override {
        const char * msg{nullptr};
        switch (item.get_action()) {
            case libdnf::transaction::TransactionItemAction::UPGRADE:
                msg = "Upgrading ";
                break;
            case libdnf::transaction::TransactionItemAction::DOWNGRADE:
                msg = "Downgrading ";
                break;
            case libdnf::transaction::TransactionItemAction::REINSTALL:
                msg = "Reinstalling ";
                break;
            case libdnf::transaction::TransactionItemAction::INSTALL:
            case libdnf::transaction::TransactionItemAction::REMOVE:
            case libdnf::transaction::TransactionItemAction::REPLACED:
                break;
            case libdnf::transaction::TransactionItemAction::REASON_CHANGE:
                throw std::logic_error(fmt::format(
                    "Unexpected action in TransactionPackage: {}",
                    static_cast<std::underlying_type_t<libdnf::base::Transaction::TransactionRunResult>>(
                        item.get_action())));
        }
        if (!msg) {
            msg = "Installing ";
        }
        new_progress_bar(static_cast<int64_t>(total), msg + item.get_package().get_full_nevra());
    }

    void install_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        multi_progress_bar.print();
    }

    void transaction_progress(uint64_t amount, [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void transaction_start(uint64_t total) override {
        new_progress_bar(static_cast<int64_t>(total), "Prepare transaction");
    }

    void transaction_stop([[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(total));
        multi_progress_bar.print();
    }

    void uninstall_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void uninstall_start(const libdnf::rpm::TransactionItem & item, uint64_t total) override {
        const char * msg{nullptr};
        if (item.get_action() == libdnf::transaction::TransactionItemAction::REMOVE ||
            item.get_action() == libdnf::transaction::TransactionItemAction::REPLACED) {
            msg = "Erasing ";
        }
        if (!msg) {
            msg = "Cleanup ";
        }
        new_progress_bar(static_cast<int64_t>(total), msg + item.get_package().get_full_nevra());
    }

    void uninstall_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        multi_progress_bar.print();
    }


    void unpack_error(const libdnf::rpm::TransactionItem & item) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR, "Unpack error: " + item.get_package().get_full_nevra());
        active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
        multi_progress_bar.print();
    }

    void cpio_error(const libdnf::rpm::TransactionItem & item) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR, "Cpio error: " + item.get_package().get_full_nevra());
        active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
        multi_progress_bar.print();
    }

    void script_error(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::ERROR,
            fmt::format(
                "Error in {} scriptlet: {} return code {}",
                script_type_to_string(type),
                to_full_nevra_string(nevra),
                return_code));
        multi_progress_bar.print();
    }

    void script_start(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::INFO,
            fmt::format("Running {} scriptlet: {}", script_type_to_string(type), to_full_nevra_string(nevra)));
        multi_progress_bar.print();
    }

    void script_stop(
        [[maybe_unused]] const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type,
        [[maybe_unused]] uint64_t return_code) override {
        active_progress_bar->add_message(
            libdnf::cli::progressbar::MessageType::INFO,
            fmt::format("Stop {} scriptlet: {}", script_type_to_string(type), to_full_nevra_string(nevra)));
        multi_progress_bar.print();
    }

    void elem_progress(
        [[maybe_unused]] const libdnf::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) override {
        //std::cout << "Element progress: " << header.get_full_nevra() << " " << amount << '/' << total << std::endl;
    }

    void verify_progress(uint64_t amount, [[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        if (is_time_to_print()) {
            multi_progress_bar.print();
        }
    }

    void verify_start([[maybe_unused]] uint64_t total) override {
        new_progress_bar(static_cast<int64_t>(total), "Verify package files");
    }

    void verify_stop([[maybe_unused]] uint64_t total) override {
        active_progress_bar->set_ticks(static_cast<int64_t>(total));
        multi_progress_bar.print();
    }

private:
    void new_progress_bar(int64_t total, const std::string & descr) {
        if (active_progress_bar &&
            active_progress_bar->get_state() != libdnf::cli::progressbar::ProgressBarState::ERROR) {
            active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        }
        auto progress_bar =
            std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(static_cast<int64_t>(total), descr);
        progress_bar->set_auto_finish(false);
        progress_bar->start();
        active_progress_bar = progress_bar.get();
        multi_progress_bar.add_bar(std::move(progress_bar));
    }

    static bool is_time_to_print() {
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

    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;

    libdnf::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf::cli::progressbar::DownloadProgressBar * active_progress_bar{nullptr};
};

std::chrono::time_point<std::chrono::steady_clock> RpmTransCB::prev_print_time = std::chrono::steady_clock::now();

static bool user_confirm_key(libdnf::ConfigMain & config, const libdnf::rpm::KeyInfo & key_info) {
    std::cout << "Importing PGP key 0x" << key_info.get_short_key_id() << std::endl;
    for (auto & user_id : key_info.get_user_ids()) {
        std::cout << " UserId     : \"" << user_id << "\"" << std::endl;
    }
    std::cout << " Fingerprint: " << key_info.get_fingerprint() << std::endl;
    std::cout << " From       : " << key_info.get_url() << std::endl;
    return libdnf::cli::utils::userconfirm::userconfirm(config);
}

}  // namespace


bool Context::check_gpg_signatures(libdnf::base::Transaction & transaction) {
    bool any_import_confirmed = false;
    std::function<bool(const libdnf::rpm::KeyInfo &)> bound_user_confirm = [&](const libdnf::rpm::KeyInfo & key_info) {
        auto import_confirmed = user_confirm_key(base.get_config(), key_info);
        if (import_confirmed) {
            any_import_confirmed = true;
        }
        return import_confirmed;
    };

    auto check_succeeded = transaction.check_gpg_signatures(bound_user_confirm);
    for (auto const & entry : transaction.get_gpg_signature_problems()) {
        std::cerr << entry << std::endl;
    }
    if (check_succeeded && any_import_confirmed) {
        std::cout << _("Keys were successfully imported.") << std::endl;
    }
    return check_succeeded;
}


void Context::download_and_run(libdnf::base::Transaction & transaction) {
    transaction.download();

    if (base.get_config().get_downloadonly_option().get_value()) {
        return;
    }

    std::cout << std::endl << "Verifying PGP signatures" << std::endl;
    if (!check_gpg_signatures(transaction)) {
        throw libdnf::cli::CommandExitError(1, M_("Signature verification failed"));
    }

    std::cout << std::endl << "Running transaction" << std::endl;

    std::string cmd_line;
    for (size_t i = 0; i < argc; ++i) {
        if (i > 0) {
            cmd_line += " ";
        }
        cmd_line += argv[i];
    }

    // Compute the total number of transaction actions (number of bars)
    // Total number of actions = number of packages in the transaction +
    //                           action of verifying package files if new package files are present in the transaction +
    //                           action of preparing transaction
    const auto & trans_packages = transaction.get_transaction_packages();
    auto num_of_actions = trans_packages.size() + 1;
    for (auto & trans_pkg : trans_packages) {
        if (libdnf::transaction::transaction_item_action_is_inbound(trans_pkg.get_action())) {
            ++num_of_actions;
            break;
        }
    }

    auto callbacks = std::make_unique<RpmTransCB>();
    callbacks->get_multi_progress_bar()->set_total_num_of_bars(num_of_actions);
    transaction.set_callbacks(std::move(callbacks));

    transaction.set_description(cmd_line);

    if (comment) {
        transaction.set_comment(comment);
    }

    auto result = transaction.run();
    std::cout << std::endl;
    if (result != libdnf::base::Transaction::TransactionRunResult::SUCCESS) {
        std::cout << "Transaction failed: " << libdnf::base::Transaction::transaction_result_to_string(result)
                  << std::endl;
        for (auto & problem : transaction.get_transaction_problems()) {
            std::cout << "  - " << problem << std::endl;
        }
        throw libdnf::cli::SilentCommandExitError(1, M_(""));
    }

    // TODO(mblaha): print a summary of successful transaction
}

libdnf::Goal * Context::get_goal(bool new_if_not_exist) {
    if (!goal && new_if_not_exist) {
        goal = std::make_unique<libdnf::Goal>(base);
    }
    return goal.get();
}

void Command::goal_resolved() {
    auto & transaction = *get_context().get_transaction();
    if (transaction.get_problems() != libdnf::GoalProblem::NO_PROBLEM) {
        throw libdnf::cli::GoalResolveError(transaction);
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

    base.get_config().get_assumeno_option().set(libdnf::Option::Priority::RUNTIME, true);
    ctx.set_quiet(true);

    base.load_config_from_file();
    base.setup();

    // optimization - disable the search for matching installed and available packages for file patterns
    if (libdnf::utils::is_file_pattern(pattern)) {
        installed = available = false;
    }

    if (installed) {
        try {
            base.get_repo_sack()->get_system_repo()->load();
            base.get_rpm_package_sack()->load_config_excludes_includes();
        } catch (...) {
            // Ignores errors when completing installed packages, other completions may still work.
        }
    }

    if (available) {
        try {
            // create rpm repositories according configuration files
            base.get_repo_sack()->create_repos_from_system_configuration();
            base.get_config().get_optional_metadata_types_option().set(
                libdnf::Option::Priority::RUNTIME, libdnf::OptionStringSet::ValueType{});

            ctx.apply_repository_setopts();

            libdnf::repo::RepoQuery enabled_repos(base);
            enabled_repos.filter_enabled(true);
            enabled_repos.filter_type(libdnf::repo::Repo::Type::AVAILABLE);
            for (auto & repo : enabled_repos.get_data()) {
                repo->set_sync_strategy(libdnf::repo::Repo::SyncStrategy::ONLY_CACHE);
                repo->get_config().get_skip_if_unavailable_option().set(libdnf::Option::Priority::RUNTIME, true);
            }

            ctx.load_repos(false);
        } catch (...) {
            // Ignores errors when completing available packages, other completions may still work.
        }
    }

    std::set<std::string> result_set;
    {
        libdnf::rpm::PackageQuery matched_pkgs_query(base);
        matched_pkgs_query.resolve_pkg_spec(
            pattern + '*', {.ignore_case = false, .with_provides = false, .with_filenames = false}, true);

        for (const auto & package : matched_pkgs_query) {
            auto [it, inserted] = result_set.insert(package.get_name());

            // Package name was already present - not inserted. There are multiple packages with the same name.
            // If requested, removes the name and inserts a full nevra for these packages.
            if (nevra_for_same_name && !inserted) {
                result_set.erase(it);
                libdnf::rpm::PackageQuery name_query(matched_pkgs_query);
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
