// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "advisory_info.hpp"

#include "../../wrappers/dbus_advisory_wrapper.hpp"

#include <libdnf5-cli/output/adapters/advisory_tmpl.hpp>
#include <libdnf5-cli/output/advisoryinfo.hpp>

#include <iostream>

namespace dnfdaemon::client {

void AdvisoryInfoCommand::process_and_print_queries(const std::vector<DbusAdvisoryWrapper> & advisories) {
    for (const auto & advisory : advisories) {
        libdnf5::cli::output::AdvisoryInfo advisory_info;
        libdnf5::cli::output::AdvisoryAdapter cli_advisory(advisory);
        advisory_info.add_advisory(cli_advisory);
        advisory_info.print();
        std::cout << std::endl;
    }
}

void AdvisoryInfoCommand::pre_configure() {
    advisory_attrs = std::vector<std::string>{
        "name",
        "title",
        "type",
        "severity",
        "status",
        "vendor",
        "description",
        "buildtime",
        "message",
        "rights",
        "collections",
        "references"};
}

}  // namespace dnfdaemon::client
