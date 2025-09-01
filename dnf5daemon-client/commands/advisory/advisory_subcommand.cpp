// Copyright Contributors to the DNF5 project.
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

#include "advisory_subcommand.hpp"

#include "../../wrappers/dbus_advisory_wrapper.hpp"
#include "context.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/conf/const.hpp>

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void AdvisorySubCommand::set_argument_parser() {
    all = std::make_unique<AdvisoryAllOption>(*this);
    available = std::make_unique<AdvisoryAvailableOption>(*this);
    installed = std::make_unique<AdvisoryInstalledOption>(*this);
    updates = std::make_unique<AdvisoryUpdatesOption>(*this);
    all->get_arg()->add_conflict_argument(*available->get_arg());
    all->get_arg()->add_conflict_argument(*installed->get_arg());
    all->get_arg()->add_conflict_argument(*updates->get_arg());
    available->get_arg()->add_conflict_argument(*installed->get_arg());
    available->get_arg()->add_conflict_argument(*updates->get_arg());
    installed->get_arg()->add_conflict_argument(*updates->get_arg());

    contains_pkgs = std::make_unique<AdvisoryContainsPkgsOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    advisory_severities = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bzs = std::make_unique<BzOption>(*this);
    advisory_cves = std::make_unique<CveOption>(*this);
    with_bz = std::make_unique<AdvisoryWithBzOption>(*this);
    with_cve = std::make_unique<AdvisoryWithCveOption>(*this);

    advisory_names = std::make_unique<AdvisoryNameArguments>(*this);
}

dnfdaemon::KeyValueMap AdvisorySubCommand::session_config() {
    dnfdaemon::KeyValueMap cfg = {};
    cfg["load_system_repo"] = sdbus::Variant(true);
    cfg["load_available_repos"] = sdbus::Variant(true);
    cfg["optional_metadata_types"] = sdbus::Variant(std::vector<std::string>{libdnf5::METADATA_TYPE_UPDATEINFO});
    return cfg;
}

void AdvisorySubCommand::run() {
    auto & ctx = get_context();

    // convert arguments to dbus call options
    dnfdaemon::KeyValueMap options = {};

    options["names"] = sdbus::Variant(advisory_names->get_value());

    // by default return available advisories
    std::string availability = "available";
    if (all->get_value()) {
        availability = "all";
    } else if (installed->get_value()) {
        availability = "installed";
    } else if (updates->get_value()) {
        availability = "updates";
    }
    options["availability"] = sdbus::Variant(availability);  // string

    // advisory types
    std::vector<std::string> advisory_types{};
    if (advisory_security->get_value()) {
        advisory_types.emplace_back("security");
    }
    if (advisory_bugfix->get_value()) {
        advisory_types.emplace_back("bugfix");
    }
    if (advisory_enhancement->get_value()) {
        advisory_types.emplace_back("enhancement");
    }
    if (advisory_newpackage->get_value()) {
        advisory_types.emplace_back("newpackage");
    }
    options["types"] = sdbus::Variant(advisory_types);  // vector<string>

    options["contains_pkgs"] = sdbus::Variant(contains_pkgs->get_value());     // vector<string>
    options["severities"] = sdbus::Variant(advisory_severities->get_value());  // vector<string>
    options["reference_bzs"] = sdbus::Variant(advisory_bzs->get_value());      // vector<string>
    options["reference_cves"] = sdbus::Variant(advisory_cves->get_value());    // vector<string>
    options["with_bz"] = sdbus::Variant(with_bz->get_value());                 // bool
    options["with_cve"] = sdbus::Variant(with_cve->get_value());               // bool

    options["advisory_attrs"] = sdbus::Variant(advisory_attrs);

    // call the server
    dnfdaemon::KeyValueMapList raw_advisories;
    ctx.session_proxy->callMethod("list")
        .onInterface(dnfdaemon::INTERFACE_ADVISORY)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options)
        .storeResultsTo(raw_advisories);

    ctx.reset_download_cb();

    std::vector<DbusAdvisoryWrapper> advisories;
    advisories.reserve(raw_advisories.size());
    for (const auto & raw_advisory : raw_advisories) {
        advisories.emplace_back(raw_advisory);
    }

    process_and_print_queries(advisories);
}

}  // namespace dnfdaemon::client
