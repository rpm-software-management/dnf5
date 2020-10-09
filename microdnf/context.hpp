/*
Copyright (C) 2019-2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MICRODNF_CONTEXT_HPP
#define MICRODNF_CONTEXT_HPP

#include "commands/command.hpp"

#include <libdnf/base/base.hpp>
#include <libdnf-cli/argument_parser.hpp>
#include <libdnf/rpm/transaction.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace microdnf {

constexpr const char * VERSION = "0.1.0";

class Context {
public:
    /// Updates the repositories metadata cache.
    /// Loads the updated metadata into rpm::RepoSack and into rpm::SolvSack.
    void load_rpm_repos(libdnf::rpm::RepoQuery & repos, libdnf::rpm::SolvSack::LoadRepoFlags flags);

    /// Select commend to execute
    void select_command(Command * cmd) { selected_command = cmd; }

    libdnf::Base base;
    std::vector<std::pair<std::string, std::string>> setopts;
    std::vector<std::unique_ptr<Command>> commands;
    Command * selected_command{nullptr};
    libdnf::cli::ArgumentParser arg_parser;

private:
    /// Updates the repository metadata cache and load it into rpm::RepoSack.
    void load_rpm_repo(libdnf::rpm::Repo & repo);
};

class RpmTransactionItem : public libdnf::rpm::TransactionItem {
public:
    enum class Actions { INSTALL, ERASE, UPGRADE, DOWNGRADE, REINSTALL };

    RpmTransactionItem(libdnf::rpm::Package pkg, Actions action) : TransactionItem(pkg), action(action) {}
    Actions get_action() const noexcept { return action; }

private:
    Actions action;
};

/// Asks the user for confirmation. The default answer is taken from the configuration.
bool userconfirm(libdnf::ConfigMain & config);

/// Downoad packages to destdir. If destdir == nullptr, packages are downloaded to the cache.
void download_packages(const std::vector<libdnf::rpm::Package> & packages, const char * dest_dir);
void download_packages(libdnf::Goal & goal, const char * dest_dir);

void run_transaction(libdnf::rpm::Transaction & transaction);

void prepare_transaction(libdnf::Goal & goal, libdnf::rpm::Transaction & ts, std::vector<std::unique_ptr<RpmTransactionItem>> & transaction_items);

}  // namespace microdnf

#endif
