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

#include "../../wrappers/dbus_advisory_wrapper.hpp"

#include <libdnf5-cli/output/adapters/advisory_tmpl.hpp>
#include <libdnf5-cli/output/advisorylist.hpp>

#include <iostream>

namespace dnfdaemon::client {

void AdvisoryListCommand::process_and_print_queries(const std::vector<DbusAdvisoryWrapper> & advisories) {
    std::vector<std::unique_ptr<libdnf5::cli::output::IAdvisoryPackage>> installed_pkgs;
    std::vector<std::unique_ptr<libdnf5::cli::output::IAdvisoryPackage>> not_installed_pkgs;

    for (const auto & advisory : advisories) {
        // TODO(mblaha): filter the output according contains_pkgs?
        // Advisories are correctly matched against contains_pkgs on the server side
        // but all their packages are returned (and printed).
        /*XXX
        std::cout << advisory.get_name() << " " << advisory.get_type() << std::endl;
        for (const auto & ref : advisory.get_references()) {
            std::cout << "  REF:" << ref.get_id() << "/" << ref.get_type() << "/" << ref.get_title() << "/" << ref.get_url() << std::endl;
        }
        */
        for (const auto & col : advisory.get_collections()) {
            for (const auto & pkg : col.get_packages()) {
                auto applicability = pkg.get_applicability();
                if (applicability == "installed") {
                    installed_pkgs.emplace_back(new libdnf5::cli::output::AdvisoryPackageAdapter(pkg));
                } else if (applicability == "available") {
                    not_installed_pkgs.emplace_back(new libdnf5::cli::output::AdvisoryPackageAdapter(pkg));
                }
            }
        }
    }

    libdnf5::cli::output::print_advisorylist_table(not_installed_pkgs, installed_pkgs);
}

void AdvisoryListCommand::pre_configure() {
    advisory_attrs = std::vector<std::string>{"name", "type", "severity", "buildtime", "collections", "references"};
}

}  // namespace dnfdaemon::client
