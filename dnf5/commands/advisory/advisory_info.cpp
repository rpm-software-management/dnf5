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

#include "advisory_info.hpp"

#include <libdnf5-cli/output/adapters/advisory.hpp>
#include <libdnf5-cli/output/advisoryinfo.hpp>
#include <libdnf5/rpm/package_query.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void AdvisoryInfoCommand::process_and_print_queries(
    Context & ctx, libdnf5::advisory::AdvisoryQuery & advisories, const std::vector<std::string> & package_specs) {
    libdnf5::rpm::PackageQuery packages(ctx.base, libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);
    if (package_specs.size() > 0) {
        packages.filter_name(package_specs, libdnf5::sack::QueryCmp::IGLOB);
    }
    if (all->get_value()) {
        packages.filter_installed();
        advisories.filter_packages(packages, libdnf5::sack::QueryCmp::LTE);
        auto advisory_query_not_installed = libdnf5::advisory::AdvisoryQuery(ctx.base);
        advisory_query_not_installed.filter_packages(packages, libdnf5::sack::QueryCmp::GT);
        advisories |= advisory_query_not_installed;
    } else if (installed->get_value()) {
        packages.filter_installed();
        advisories.filter_packages(packages, libdnf5::sack::QueryCmp::LTE);
    } else if (updates->get_value()) {
        packages.filter_upgradable();
        advisories.filter_packages(packages, libdnf5::sack::QueryCmp::GT);
    } else {  // available is the default
        libdnf5::rpm::PackageQuery installed_packages(ctx.base);
        installed_packages.filter_installed();
        installed_packages.filter_latest_evr();

        add_running_kernel_packages(ctx.base, installed_packages);

        if (package_specs.size() > 0) {
            installed_packages.filter_name(package_specs, libdnf5::sack::QueryCmp::IGLOB);
        }

        advisories.filter_packages(installed_packages, libdnf5::sack::QueryCmp::GT);
    }

    for (auto advisory : advisories) {
        libdnf5::cli::output::AdvisoryInfo advisory_info;
        output::AdvisoryAdapter cli_advisory(advisory);
        advisory_info.add_advisory(cli_advisory);
        advisory_info.print();
        std::cout << std::endl;
    }
}

}  // namespace dnf5
