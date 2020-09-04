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

#include "argument_parser.hpp"
#include "commands/command.hpp"

#include <libdnf/base/base.hpp>
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
    ArgumentParser arg_parser;

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

/// Downoad packages to destdir. If destdir == nullptr, packages are downloaded to the cache.
void download_packages(libdnf::rpm::PackageSet & package_set, const char * dest_dir);

void run_transaction(libdnf::rpm::Transaction & transaction);

}  // namespace microdnf

#endif
