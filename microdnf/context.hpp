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

#ifndef MICRODNF_CONTEXT_HPP
#define MICRODNF_CONTEXT_HPP


#include <libdnf-cli/argument_parser.hpp>
#include <libdnf-cli/session.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/base/transaction.hpp>
#include <libdnf/rpm/transaction.hpp>
#include <libdnf/utils/span.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace microdnf {

constexpr const char * VERSION = "0.1.0";

class Context : public libdnf::cli::session::Session {
public:
    void set_cache_dir();

    void apply_repository_setopts();

    /// Updates the repositories metadata cache.
    /// Loads the updated metadata into rpm::RepoSack and into rpm::PackageSack.
    void load_rpm_repos(libdnf::repo::RepoQuery & repos, libdnf::rpm::PackageSack::LoadRepoFlags flags = libdnf::rpm::PackageSack::LoadRepoFlags::ALL);

    libdnf::Base base;
    std::vector<std::pair<std::string, std::string>> setopts;

    /// Gets program arguments.
    libdnf::Span<const char * const> get_prg_arguments() const { return prg_args; }

    /// Stores reference to program arguments.
    void set_prg_arguments(size_t argc, const char * const * argv) { prg_args = {argv, argc}; }

    /// Gets user comment.
    const char * get_comment() const noexcept { return comment; }

    /// Stores pointer to user comment.
    void set_comment(const char * comment) noexcept { this->comment = comment; }

    /// Downloads transaction packages, creates the history DB transaction and
    /// rpm transaction and runs it.
    void download_and_run(libdnf::base::Transaction & transaction);

    /// Set to true to suppresses messages notifying about the current state or actions of microdnf.
    void set_quiet(bool quiet) { this->quiet = quiet; }

    bool get_quiet() const { return quiet; }

private:
    /// Updates the repository metadata cache and load it into rpm::RepoSack.
    void load_rpm_repo(libdnf::repo::Repo & repo);

    /// Creates, initializes and returns new database transaction.
    libdnf::transaction::TransactionWeakPtr new_db_transaction();

    /// If quiet mode is not active, it will print `msg` to standard output.
    void print_info(const char * msg);

    /// Refers to program arguments.
    libdnf::Span<const char * const> prg_args;

    /// Points to user comment.
    const char * comment{nullptr};

    bool quiet{false};
};

class RpmTransactionItem : public libdnf::rpm::TransactionItem {
public:
    enum class Actions { INSTALL, ERASE, UPGRADE, DOWNGRADE, REINSTALL };

    RpmTransactionItem(const libdnf::base::TransactionPackage & tspkg);
    Actions get_action() const noexcept { return action; }

private:
    Actions action;
};

/// Asks the user for confirmation. The default answer is taken from the configuration.
bool userconfirm(libdnf::ConfigMain & config);

/// Downoad packages to destdir. If destdir == nullptr, packages are downloaded to the cache.
void download_packages(const std::vector<libdnf::rpm::Package> & packages, const char * dest_dir);
void download_packages(libdnf::base::Transaction & transaction, const char * dest_dir);

void run_transaction(libdnf::rpm::Transaction & transaction);

/// Returns the names of matching installed packages.
/// If `nevra_for_same_name` is true, it returns a full nevra for packages with the same name.
std::vector<std::string> match_installed_pkgs(Context & ctx, const std::string & pattern, bool nevra_for_same_name);

}  // namespace microdnf

#endif
