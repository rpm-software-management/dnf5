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

#include "advisory.hpp"

#include "../../advisory.hpp"
#include "../../dbus.hpp"
#include "../../utils.hpp"
#include "utils.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <sdbus-c++/sdbus-c++.h>

namespace dnfdaemon {

void Advisory::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        INTERFACE_ADVISORY,
        "list",
        "a{sv}",
        {"options"},
        "aa{sv}",
        {"advisories"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Advisory::list, call, session.session_locale);
        });
}

libdnf5::advisory::AdvisoryQuery Advisory::advisory_query_from_options(
    libdnf5::Base & base, const KeyValueMap & options) {
    auto opt_types = key_value_map_get<std::vector<std::string>>(options, "types", {});
    auto opt_severities = key_value_map_get<std::vector<std::string>>(options, "severities", {});
    auto opt_names = key_value_map_get<std::vector<std::string>>(options, "names", {});
    auto opt_reference_bzs = key_value_map_get<std::vector<std::string>>(options, "reference_bzs", {});
    auto opt_reference_cves = key_value_map_get<std::vector<std::string>>(options, "reference_cves", {});

    auto advisories = libdnf5::advisory::AdvisoryQuery(base);

    if (!opt_types.empty() || !opt_severities.empty() || !opt_names.empty() || !opt_reference_bzs.empty() ||
        !opt_reference_cves.empty()) {
        advisories.clear();
        // Filter by advisory name
        if (!opt_names.empty()) {
            auto advisories_names = libdnf5::advisory::AdvisoryQuery(base);
            advisories_names.filter_name(opt_names);
            advisories |= advisories_names;
        }

        // Filter by advisory type
        if (!opt_types.empty()) {
            auto advisories_types = libdnf5::advisory::AdvisoryQuery(base);
            advisories_types.filter_type(opt_types);
            advisories |= advisories_types;
        }

        // Filter by advisory severity
        if (!opt_severities.empty()) {
            auto advisories_severities = libdnf5::advisory::AdvisoryQuery(base);
            advisories_severities.filter_severity(opt_severities);
            advisories |= advisories_severities;
        }

        // Filter by advisory bz
        if (!opt_reference_bzs.empty()) {
            auto advisories_bzs = libdnf5::advisory::AdvisoryQuery(base);
            advisories_bzs.filter_reference(opt_reference_bzs, {"bugzilla"});
            advisories |= advisories_bzs;
        }

        // Filter by advisory cve
        if (!opt_reference_cves.empty()) {
            auto advisories_cves = libdnf5::advisory::AdvisoryQuery(base);
            advisories_cves.filter_reference(opt_reference_cves, {"cve"});
            advisories |= advisories_cves;
        }
    }

    if (key_value_map_get<bool>(options, "with_bz", false)) {
        advisories.filter_reference("*", {"bugzilla"}, libdnf5::sack::QueryCmp::IGLOB);
    }
    if (key_value_map_get<bool>(options, "with_cve", false)) {
        advisories.filter_reference("*", {"cve"}, libdnf5::sack::QueryCmp::IGLOB);
    }

    libdnf5::rpm::PackageQuery package_query(base, libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);
    auto opt_contains_pkgs = key_value_map_get<std::vector<std::string>>(options, "contains_pkgs", {});
    if (!opt_contains_pkgs.empty()) {
        package_query.filter_name(opt_contains_pkgs, libdnf5::sack::QueryCmp::IGLOB);
    }

    auto opt_availability = key_value_map_get<std::string>(options, "availability", "");
    std::transform(opt_availability.begin(), opt_availability.end(), opt_availability.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    if (opt_availability == "all") {
        package_query.filter_installed();
        auto advisories_not_installed(advisories);
        advisories.filter_packages(package_query, libdnf5::sack::QueryCmp::LTE);
        // TODO(mblaha): add advisories.filter_packages(package_query), without cmp,
        // to filter advisories with matching name.arch? Instead of unioning LTE
        // and GT results.
        advisories_not_installed.filter_packages(package_query, libdnf5::sack::QueryCmp::GT);
        advisories |= advisories_not_installed;
    } else if (opt_availability == "installed") {
        package_query.filter_installed();
        advisories.filter_packages(package_query, libdnf5::sack::QueryCmp::LTE);
    } else if (opt_availability == "updates") {
        package_query.filter_upgradable();
        advisories.filter_packages(package_query, libdnf5::sack::QueryCmp::GT);
    } else {
        libdnf5::rpm::PackageQuery installed_package_query(base);
        installed_package_query.filter_installed();
        installed_package_query.filter_latest_evr();
        auto kernel = base.get_rpm_package_sack()->get_running_kernel();
        if (kernel.get_id().id > 0) {
            libdnf5::rpm::PackageQuery kernel_query(base);
            kernel_query.filter_sourcerpm({kernel.get_sourcerpm()});
            kernel_query.filter_installed();
            installed_package_query |= kernel_query;
        }
        if (!opt_contains_pkgs.empty()) {
            installed_package_query.filter_name(opt_contains_pkgs, libdnf5::sack::QueryCmp::IGLOB);
        }
        advisories.filter_packages(installed_package_query, libdnf5::sack::QueryCmp::GT);
    }

    return advisories;
}

sdbus::MethodReply Advisory::list(sdbus::MethodCall & call) {
    // read options from dbus call
    KeyValueMap options;
    call >> options;

    session.fill_sack();

    auto base = session.get_base();
    auto advisory_query = advisory_query_from_options(*base, options);
    auto opt_attrs = key_value_map_get<std::vector<std::string>>(options, "advisory_attrs", {});

    // to decide whether particular advisory package is installed / available / unrelated
    // to the system we need the latest versions of each installed n.a
    libdnf5::rpm::PackageQuery installed_pkgs(*base);
    installed_pkgs.filter_installed();
    installed_pkgs.filter_latest_evr();
    // map installed na -> package
    std::unordered_map<std::string, libdnf5::rpm::Package> installed_versions;
    for (const auto & pkg : installed_pkgs) {
        installed_versions.emplace(pkg.get_na(), std::move(pkg));
    }

    KeyValueMapList advisories;

    for (const auto & advisory : advisory_query) {
        advisories.emplace_back(advisory_to_map(advisory, opt_attrs, installed_versions));
    }

    auto reply = call.createReply();
    reply << advisories;
    return reply;
}

}  // namespace dnfdaemon
