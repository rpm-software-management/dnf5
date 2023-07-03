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

#include "advisory_list.hpp"

#include <libdnf5-cli/output/advisorylist.hpp>
#include <libdnf5/advisory/advisory_package.hpp>
#include <libdnf5/rpm/package_query.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void AdvisoryListCommand::process_and_print_queries(
    [[maybe_unused]] Context & ctx,
    libdnf5::advisory::AdvisoryQuery & advisories,
    libdnf5::rpm::PackageQuery & packages) {
    std::vector<libdnf5::advisory::AdvisoryPackage> installed_pkgs;
    std::vector<libdnf5::advisory::AdvisoryPackage> not_installed_pkgs;

    if (all->get_value()) {
        packages.filter_installed();
        installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf5::sack::QueryCmp::LTE);
        not_installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf5::sack::QueryCmp::GT);
    } else if (installed->get_value()) {
        packages.filter_installed();
        installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf5::sack::QueryCmp::LTE);
    } else if (updates->get_value()) {
        //TODO(amatej): all advisory commands with --updates should respect obsoletes and upgrades when noarch is involved,
        //              filed as: https://issues.redhat.com/browse/RHELPLAN-133820
        packages.filter_upgradable();
        not_installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf5::sack::QueryCmp::GT);
    } else {  // available is the default
        packages.filter_installed();
        packages.filter_latest_evr();

        not_installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf5::sack::QueryCmp::GT);
    }

    if (with_bz->get_value()) {
        libdnf5::cli::output::print_advisorylist_references_table(not_installed_pkgs, installed_pkgs, "bugzilla");
    } else if (with_cve->get_value()) {
        libdnf5::cli::output::print_advisorylist_references_table(not_installed_pkgs, installed_pkgs, "cve");
    } else {
        libdnf5::cli::output::print_advisorylist_table(not_installed_pkgs, installed_pkgs);
    }
}

}  // namespace dnf5
