// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "libdnf5-cli/output/status.hpp"

#include "key_value_table.hpp"
#include "utils/string.hpp"

#include <libdnf5-cli/utils/units.hpp>
#include <libdnf5/comps/environment/query.hpp>
#include <libdnf5/repo/repo.hpp>
#include <libdnf5/repo/repo_query.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/transaction/transaction_history.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <sstream>

namespace libdnf5::cli::output {

void print_status_table(libdnf5::Base & base, const libdnf5::comps::GroupQuery & query) {
    KeyValueTable output_table;

    // Groups
    libdnf5::comps::GroupQuery query_installed(query);
    query_installed.filter_installed(true);
    const auto groups_installed = query_installed.size();
    auto groups_row = output_table.add_line(_("Groups"), query.size(), nullptr);
    output_table.add_line(_("installed"), groups_installed, nullptr, groups_row);
    output_table.add_line(_("available"), query.size() - groups_installed, nullptr, groups_row);

    // Environments
    libdnf5::comps::EnvironmentQuery env_query(base);
    libdnf5::comps::EnvironmentQuery env_installed(env_query);
    env_installed.filter_installed(true);
    const auto envs_installed = env_installed.size();
    auto envs_row = output_table.add_line(_("Environments"), env_query.size(), nullptr);
    output_table.add_line(_("installed"), envs_installed, nullptr, envs_row);
    output_table.add_line(_("available"), env_query.size() - envs_installed, nullptr, envs_row);

    // Packages
    libdnf5::rpm::PackageQuery pkg_installed(base);
    pkg_installed.filter_installed();
    const auto pkgs_installed = pkg_installed.size();

    libdnf5::rpm::PackageQuery pkg_available(base);
    pkg_available.filter_available();
    const auto pkgs_available = pkg_available.size();

    libdnf5::rpm::PackageQuery pkg_user(base);
    pkg_user.filter_userinstalled();
    const auto pkgs_user = pkg_user.size();

    auto pkgs_row = output_table.add_line(_("Packages"), pkgs_installed + pkgs_available, nullptr);
    output_table.add_line(_("installed"), pkgs_installed, nullptr, pkgs_row);
    output_table.add_line(_("available"), pkgs_available, nullptr, pkgs_row);
    output_table.add_line(_("user-installed"), pkgs_user, nullptr, pkgs_row);

    // Installed size
    unsigned long long total_install_size = 0;
    for (const auto & pkg : pkg_installed) {
        total_install_size += pkg.get_install_size();
    }
    auto [size_value, size_unit] = libdnf5::cli::utils::units::to_size(static_cast<int64_t>(total_install_size));
    std::ostringstream size_str;
    size_str << std::fixed << std::setprecision(1) << size_value << " " << size_unit;
    output_table.add_line(_("Installed Size"), size_str.str(), nullptr);

    // Repositories
    libdnf5::repo::RepoQuery repo_query(base);
    repo_query.filter_enabled(true);
    repo_query.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);
    output_table.add_line(_("Enabled Repositories"), repo_query.size(), nullptr);

    // Last transaction
    try {
        libdnf5::transaction::TransactionHistory history(base);
        auto ids = history.list_transaction_ids();
        if (!ids.empty()) {
            auto transactions = history.list_transactions({ids.back()});
            if (!transactions.empty()) {
                auto dt_end = transactions.front().get_dt_end();
                if (dt_end > 0) {
                    output_table.add_line(_("Last Transaction"), libdnf5::utils::string::format_epoch(dt_end), nullptr);
                }
            }
        }
    } catch (const std::exception &) {
    }

    output_table.print();
}

}  // namespace libdnf5::cli::output
