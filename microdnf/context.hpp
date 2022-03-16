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

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace microdnf {

constexpr const char * VERSION = "0.1.0";

class Context : public libdnf::cli::session::Session {
public:
    void apply_repository_setopts();

    /// Sets callbacks for repositories and loads them, updating metadata if necessary.
    void load_repos(bool load_system, libdnf::repo::Repo::LoadFlags flags = libdnf::repo::Repo::LoadFlags::ALL);

    /// Adds packages from path-defined files to the command line repository.
    /// Returns the added Package objects.
    std::vector<libdnf::rpm::Package> add_cmdline_packages(
        const std::vector<std::string> & packages_paths, std::vector<std::string> & error_messages);

    libdnf::Base base;
    std::vector<std::pair<std::string, std::string>> setopts;
    std::vector<std::string> enable_plugins_patterns;
    std::vector<std::string> disable_plugins_patterns;

    /// Stores reference to program arguments.
    void set_prg_arguments(size_t argc, const char * const * argv) {
        this->argc = argc;
        this->argv = argv;
    }

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
    /// If quiet mode is not active, it will print `msg` to standard output.
    void print_info(const char * msg);

    /// Program arguments.
    size_t argc{0};
    const char * const * argv{nullptr};

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

/// Parses the items in the `specs` array and adds the individual items to the appropriate vector.
/// Duplicate items are ignored.
void parse_add_specs(
    int specs_count,
    const char * const specs[],
    std::vector<std::string> & pkg_specs,
    std::vector<std::string> & filepaths);

/// Returns the names of matching installed packages.
/// If `nevra_for_same_name` is true, it returns a full nevra for packages with the same name.
std::vector<std::string> match_installed_pkgs(Context & ctx, const std::string & pattern, bool nevra_for_same_name);

/// Returns the names of matching available packages.
std::vector<std::string> match_available_pkgs(Context & ctx, const std::string & pattern);

}  // namespace microdnf

#endif
