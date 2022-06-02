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

#include "libdnf-cli/output/advisorylist.hpp"

#include <libdnf/advisory/advisory_package.hpp>
#include <libdnf/rpm/package_query.hpp>

namespace dnf5 {

using namespace libdnf::cli;

void AdvisoryListCommand::process_and_print_queries(
    Context & ctx, libdnf::advisory::AdvisoryQuery & advisories, libdnf::rpm::PackageQuery & packages) {
    std::vector<libdnf::advisory::AdvisoryPackage> installed_pkgs;
    std::vector<libdnf::advisory::AdvisoryPackage> not_installed_pkgs;

    if (all->get_value()) {
        packages.filter_installed();
        installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf::sack::QueryCmp::LTE);
        not_installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf::sack::QueryCmp::GT);
    } else if (installed->get_value()) {
        packages.filter_installed();
        installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf::sack::QueryCmp::LTE);
    } else if (updates->get_value()) {
        packages.filter_upgradable();
        not_installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf::sack::QueryCmp::GT);
    } else {  // available is the default
        packages.filter_installed();
        packages.filter_latest_evr();

        add_running_kernel_packages(ctx.base, packages);

        not_installed_pkgs = advisories.get_advisory_packages_sorted(packages, libdnf::sack::QueryCmp::GT);
    }

    if (with_bz->get_value()) {
        libdnf::cli::output::print_advisorylist_references_table(not_installed_pkgs, installed_pkgs, "bugzilla");
    } else if (with_cve->get_value()) {
        libdnf::cli::output::print_advisorylist_references_table(not_installed_pkgs, installed_pkgs, "cve");
    } else {
        libdnf::cli::output::print_advisorylist_table(not_installed_pkgs, installed_pkgs);
    }
}

}  // namespace dnf5
