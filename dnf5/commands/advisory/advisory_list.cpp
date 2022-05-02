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

#include "dnf5/context.hpp"

#include "libdnf-cli/output/advisorylist.hpp"

#include <libdnf/advisory/advisory_package.hpp>
#include <libdnf/rpm/package_query.hpp>

#include <filesystem>
#include <fstream>


namespace dnf5 {


using namespace libdnf::cli;


AdvisoryListCommand::AdvisoryListCommand(Command & parent) : AdvisorySubCommand(parent, "list", _("List advisories")) {}

void AdvisoryListCommand::process_and_print_queries(
    Context & ctx, libdnf::advisory::AdvisoryQuery & advisories, libdnf::rpm::PackageQuery & packages) {
    std::vector<libdnf::advisory::AdvisoryPackage> installed_pkgs;
    std::vector<libdnf::advisory::AdvisoryPackage> not_installed_pkgs;

    if (all->get_value()) {
        packages.filter_installed();
        installed_pkgs = advisories.get_advisory_packages(packages, libdnf::sack::QueryCmp::LTE);
        not_installed_pkgs = advisories.get_advisory_packages(packages, libdnf::sack::QueryCmp::GT);
    } else if (installed->get_value()) {
        packages.filter_installed();
        installed_pkgs = advisories.get_advisory_packages(packages, libdnf::sack::QueryCmp::LTE);
    } else if (updates->get_value()) {
        packages.filter_upgradable();
        not_installed_pkgs = advisories.get_advisory_packages(packages, libdnf::sack::QueryCmp::GT);
    } else {  // available is the default
        packages.filter_installed();
        packages.filter_latest_evr();

        not_installed_pkgs = advisories.get_advisory_packages(packages, libdnf::sack::QueryCmp::GT);
    }

    libdnf::cli::output::print_advisorylist_table(not_installed_pkgs, installed_pkgs);
}

}  // namespace dnf5
