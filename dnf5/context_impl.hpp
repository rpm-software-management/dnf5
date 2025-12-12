// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_CONTEXT_PRIVATE_HPP
#define DNF5_CONTEXT_PRIVATE_HPP

#include "dnf5/context.hpp"
#include "plugins.hpp"

namespace dnf5 {

class Context::Impl {
public:
    explicit Impl(Context & owner, std::vector<std::unique_ptr<libdnf5::Logger>> && loggers)
        : owner(owner),
          base(std::move(loggers)),
          plugins(std::make_unique<Plugins>(owner)) {}

    void apply_repository_setopts();

    void update_repo_metadata_from_specs(const std::vector<std::string> & pkg_specs);

    void update_repo_metadata_from_advisory_options(
        const std::vector<std::string> & names,
        bool security,
        bool bugfix,
        bool enhancement,
        bool newpackage,
        const std::vector<std::string> & severity,
        const std::vector<std::string> & bzs,
        const std::vector<std::string> & cves);

    /// Sets callbacks for repositories and loads them, updating metadata if necessary.
    void load_repos(bool load_system, bool load_available);

    void store_offline(libdnf5::base::Transaction & transaction);

    const char * get_comment() const noexcept { return comment; }

    void set_comment(const char * comment) noexcept { this->comment = comment; }

    std::string get_cmdline() { return cmdline; }

    void set_cmdline(std::string & cmdline) { this->cmdline = cmdline; }

    void download_and_run(libdnf5::base::Transaction & transaction);

    void set_quiet(bool quiet) { this->quiet = quiet; }

    bool get_quiet() const { return quiet; }

    void set_dump_main_config(bool enable) { this->dump_main_config = enable; }

    bool get_dump_main_config() const { return dump_main_config; }

    void set_dump_repo_config_id_list(const std::vector<std::string> & repo_id_list) {
        this->dump_repo_config_id_list = repo_id_list;
    }

    const std::vector<std::string> & get_dump_repo_config_id_list() const { return dump_repo_config_id_list; }

    void set_dump_variables(bool enable) { this->dump_variables = enable; }

    bool get_dump_variables() const { return dump_variables; }

    void set_show_new_leaves(bool show_new_leaves) { this->show_new_leaves = show_new_leaves; }

    bool get_show_new_leaves() const { return show_new_leaves; }

    Plugins & get_plugins() { return *plugins; }

    libdnf5::Goal * get_goal(bool new_if_not_exist);

    void set_transaction(libdnf5::base::Transaction && transaction) {
        this->transaction = std::make_unique<libdnf5::base::Transaction>(std::move(transaction));
    }

    libdnf5::base::Transaction * get_transaction() { return transaction.get(); }

    void set_load_system_repo(bool on) { load_system_repo = on; }
    bool get_load_system_repo() const noexcept { return load_system_repo; }

    void set_load_available_repos(LoadAvailableRepos which) { load_available_repos = which; }
    LoadAvailableRepos get_load_available_repos() const noexcept { return load_available_repos; }

    void print_output(std::string_view msg) const;
    void print_info(std::string_view msg) const;
    void print_error(std::string_view msg) const;

    void set_output_stream(std::ostream & new_output_stream) { out_stream = new_output_stream; }
    void set_error_stream(std::ostream & new_error_stream) { err_stream = new_error_stream; }

    void set_transaction_store_path(std::filesystem::path path) { transaction_store_path = path; }
    const std::filesystem::path & get_transaction_store_path() const { return transaction_store_path; }

    void set_should_store_offline(bool should_store_offline) { this->should_store_offline = should_store_offline; }
    bool get_should_store_offline() const { return should_store_offline; }

    void set_json_output_requested(bool json_output) {
        this->json_output = json_output;
        if (json_output) {
            set_quiet(true);
        }
    }
    bool get_json_output_requested() const { return json_output; }

    void set_create_repos(bool create_repos) { this->create_repos = create_repos; }
    bool get_create_repos() const { return create_repos; }

    libdnf5::Base & get_base() { return base; };

    std::vector<std::pair<std::string, std::string>> & get_setopts() { return setopts; }
    const std::vector<std::pair<std::string, std::string>> & get_setopts() const { return setopts; }

    std::vector<std::pair<std::string, std::string>> & get_repos_from_path() { return repos_from_path; }
    const std::vector<std::pair<std::string, std::string>> & get_repos_from_path() const { return repos_from_path; }

    std::vector<std::pair<std::vector<std::string>, bool>> & get_libdnf_plugins_enablement() {
        return libdnf_plugins_enablement;
    }
    const std::vector<std::pair<std::vector<std::string>, bool>> & get_libdnf_plugins_enablement() const {
        return libdnf_plugins_enablement;
    }

    void set_show_version(bool enable) { this->show_version = enable; }
    bool get_show_version() const { return this->show_version; }

private:
    Context & owner;

    std::filesystem::path transaction_store_path;

    libdnf5::Base base;
    std::vector<std::pair<std::string, std::string>> setopts;
    std::vector<std::pair<std::string, std::string>> repos_from_path;

    /// list of lists of libdnf plugin names (global patterns) that we want to enable (true) or disable (false)
    std::vector<std::pair<std::vector<std::string>, bool>> libdnf_plugins_enablement;

    std::string cmdline;

    /// Points to user comment.
    const char * comment{nullptr};

    bool should_store_offline = false;
    bool json_output = false;
    bool create_repos = true;

    bool quiet{false};
    bool dump_main_config{false};
    std::vector<std::string> dump_repo_config_id_list;
    bool dump_variables{false};
    bool show_new_leaves{false};
    std::string get_cmd_line();

    std::reference_wrapper<std::ostream> out_stream = std::cout;
    std::reference_wrapper<std::ostream> err_stream = std::cerr;

    std::unique_ptr<Plugins> plugins;
    std::unique_ptr<libdnf5::Goal> goal;
    std::unique_ptr<libdnf5::base::Transaction> transaction;

    bool load_system_repo{false};
    LoadAvailableRepos load_available_repos{LoadAvailableRepos::NONE};
    bool show_version{false};
};

}  // namespace dnf5

#endif
