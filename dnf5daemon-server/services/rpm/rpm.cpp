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

#include "rpm.hpp"

#include "dbus.hpp"
#include "package.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>
#include <vector>


void Rpm::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM,
        "distro_sync",
        "asa{sv}",
        {"pkg_specs", "options"},
        "",
        {},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::distro_sync, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM,
        "downgrade",
        "asa{sv}",
        {"pkg_specs", "options"},
        "",
        {},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::downgrade, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM,
        "list",
        "a{sv}",
        {"options"},
        "aa{sv}",
        {"packages"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::list, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM,
        "install",
        "asa{sv}",
        {"pkg_specs", "options"},
        "",
        {},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::install, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM,
        "upgrade",
        "asa{sv}",
        {"pkg_specs", "options"},
        "",
        {},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::upgrade, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM,
        "reinstall",
        "asa{sv}",
        {"pkg_specs", "options"},
        "",
        {},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::reinstall, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_RPM,
        "remove",
        "asa{sv}",
        {"pkg_specs", "options"},
        "",
        {},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Rpm::remove, call, session.session_locale);
        });

    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_ELEM_PROGRESS,
        "ostt",
        {"session_object_path", "nevra", "processed", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_ACTION_START,
        "osut",
        {"session_object_path", "nevra", "action", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_ACTION_PROGRESS,
        "ostt",
        {"session_object_path", "nevra", "processed", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_ACTION_STOP,
        "ost",
        {"session_object_path", "nevra", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_START,
        "osu",
        {"session_object_path", "nevra", "scriptlet_type"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_STOP,
        "osut",
        {"session_object_path", "nevra", "scriptlet_type", "return_code"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_ERROR,
        "osut",
        {"session_object_path", "nevra", "scriptlet_type", "return_code"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_START, "ot", {"session_object_path", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_VERIFY_PROGRESS,
        "ott",
        {"session_object_path", "processed", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_STOP, "ot", {"session_object_path", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_START,
        "ot",
        {"session_object_path", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_PROGRESS,
        "ott",
        {"session_object_path", "processed", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_STOP,
        "ot",
        {"session_object_path", "total"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_UNPACK_ERROR, "os", {"session_object_path", "nevra"});
}

std::vector<std::string> get_filter_patterns(dnfdaemon::KeyValueMap options, const std::string & option) {
    auto filter_patterns = dnfdaemon::key_value_map_get<std::vector<std::string>>(options, option);
    if (filter_patterns.empty()) {
        throw sdbus::Error(dnfdaemon::ERROR, fmt::format("\"{}\" option expected an argument.", option));
    }
    return filter_patterns;
}

libdnf5::rpm::PackageQuery resolve_nevras(libdnf5::rpm::PackageQuery base_query, std::vector<std::string> nevras) {
    libdnf5::rpm::PackageQuery result(base_query.get_base(), libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
    libdnf5::ResolveSpecSettings settings{.with_provides = false, .with_filenames = false, .with_binaries = false};
    for (const auto & nevra : nevras) {
        libdnf5::rpm::PackageQuery nevra_query(base_query);
        nevra_query.resolve_pkg_spec(nevra, settings, false);
        result |= nevra_query;
    }
    return result;
}

sdbus::MethodReply Rpm::list(sdbus::MethodCall & call) {
    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    session.fill_sack();
    auto base = session.get_base();

    // add potential command-line packages
    std::vector<libdnf5::rpm::Package> cmdline_packages;
    std::vector<std::string> patterns =
        key_value_map_get<std::vector<std::string>>(options, "patterns", std::vector<std::string>{});
    for (auto & [path, package] : base->get_repo_sack()->add_cmdline_packages(patterns)) {
        cmdline_packages.push_back(std::move(package));
    }

    std::string scope = key_value_map_get<std::string>(options, "scope", "all");
    // start with all packages
    libdnf5::sack::ExcludeFlags flags = libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES;
    if (scope != "upgrades" && scope != "upgradable") {
        flags = flags | libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK;
    }
    libdnf5::rpm::PackageQuery query(*base, flags);

    // toplevel filtering - the scope
    // TODO(mblaha): support for other possible scopes?
    //     userinstalled, duplicates, unneeded, extras, installonly, recent, unsatisfied
    if (scope == "installed") {
        query.filter_installed();
    } else if (scope == "available") {
        query.filter_available();
    } else if (scope == "upgrades") {
        query.filter_upgrades();
    } else if (scope == "upgradable") {
        query.filter_upgradable();
    } else if (scope == "all") {
        // the query already contains all packages
    } else {
        throw sdbus::Error(dnfdaemon::ERROR, fmt::format("Unsupported scope for package filtering \"{}\".", scope));
    }

    // applying patterns filtering early can increase performance of slow
    // what* filters by reducing the size of their base query

    if (patterns.size() > 0) {
        libdnf5::rpm::PackageQuery result(*base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
        for (const auto & pkg : cmdline_packages) {
            if (query.contains(pkg)) {
                result.add(pkg);
            }
        }
        // packages matching flags
        bool with_src = key_value_map_get<bool>(options, "with_src", true);
        libdnf5::ResolveSpecSettings settings{
            .ignore_case = key_value_map_get<bool>(options, "icase", true),
            .with_nevra = key_value_map_get<bool>(options, "with_nevra", true),
            .with_provides = key_value_map_get<bool>(options, "with_provides", true),
            .with_filenames = key_value_map_get<bool>(options, "with_filenames", true),
            .with_binaries = key_value_map_get<bool>(options, "with_binaries", true)};
        for (auto & pattern : patterns) {
            libdnf5::rpm::PackageQuery package_query(query);
            package_query.resolve_pkg_spec(pattern, settings, with_src);
            result |= package_query;
        }
        query = result;
    }

    // then apply specific filters
    if (options.find("arch") != options.end()) {
        query.filter_arch(key_value_map_get<std::vector<std::string>>(options, "arch"));
    }
    if (options.find("repo") != options.end()) {
        query.filter_repo_id(key_value_map_get<std::vector<std::string>>(options, "repo"));
    }
    if (options.find("whatprovides") != options.end()) {
        auto filter_patterns = get_filter_patterns(options, "whatprovides");
        libdnf5::rpm::PackageQuery reldeps_query(query);
        reldeps_query.filter_provides(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        if (!reldeps_query.empty()) {
            query = reldeps_query;
        } else {
            query.filter_file(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        }
    }
    if (options.find("whatobsoletes") != options.end()) {
        query.filter_obsoletes(get_filter_patterns(options, "whatobsoletes"), libdnf5::sack::QueryCmp::GLOB);
    }
    if (options.find("whatconflicts") != options.end()) {
        auto filter_patterns = get_filter_patterns(options, "whatconflicts");

        libdnf5::rpm::PackageQuery by_package_query(query);
        by_package_query.filter_conflicts(resolve_nevras(query, filter_patterns));

        query.filter_conflicts(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        query |= by_package_query;
    }
    if (options.find("whatdepends") != options.end()) {
        // TODO(mblaha): support for `exactdeps/alldeps` and `recursive` options
        auto filter_patterns = get_filter_patterns(options, "whatdepends");
        auto resolved_nevras = resolve_nevras(query, filter_patterns);
        libdnf5::rpm::PackageQuery by_package_query(query);
        libdnf5::rpm::PackageQuery by_glob_query(query);
        libdnf5::rpm::PackageQuery dep_query(query.get_base(), libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);

        // requires
        by_glob_query.filter_requires(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        dep_query |= by_glob_query;
        by_package_query.filter_requires(resolved_nevras);
        dep_query |= by_package_query;
        // recommends
        by_glob_query = query;
        by_glob_query.filter_recommends(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        dep_query |= by_glob_query;
        by_package_query = query;
        by_package_query.filter_recommends(resolved_nevras);
        dep_query |= by_package_query;
        // enhances
        by_glob_query = query;
        by_glob_query.filter_enhances(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        dep_query |= by_glob_query;
        by_package_query = query;
        by_package_query.filter_enhances(resolved_nevras);
        dep_query |= by_package_query;
        // supplements
        by_glob_query = query;
        by_glob_query.filter_supplements(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        dep_query |= by_glob_query;
        by_package_query = query;
        by_package_query.filter_supplements(resolved_nevras);
        dep_query |= by_package_query;
        // suggests
        by_glob_query = query;
        by_glob_query.filter_suggests(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        dep_query |= by_glob_query;
        by_package_query = query;
        by_package_query.filter_suggests(resolved_nevras);
        dep_query |= by_package_query;

        query = dep_query;
    }
    if (options.find("whatrequires") != options.end()) {
        // TODO(mblaha): support for `exactdeps/alldeps` and `recursive` options
        auto filter_patterns = get_filter_patterns(options, "whatrequires");

        libdnf5::rpm::PackageQuery by_package_query(query);
        by_package_query.filter_requires(resolve_nevras(query, filter_patterns));

        query.filter_requires(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        query |= by_package_query;
    }
    if (options.find("whatrecommends") != options.end()) {
        auto filter_patterns = get_filter_patterns(options, "whatrecommends");

        libdnf5::rpm::PackageQuery by_package_query(query);
        by_package_query.filter_recommends(resolve_nevras(query, filter_patterns));

        query.filter_recommends(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        query |= by_package_query;
    }
    if (options.find("whatenhances") != options.end()) {
        auto filter_patterns = get_filter_patterns(options, "whatenhances");

        libdnf5::rpm::PackageQuery by_package_query(query);
        by_package_query.filter_enhances(resolve_nevras(query, filter_patterns));

        query.filter_enhances(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        query |= by_package_query;
    }
    if (options.find("whatsuggests") != options.end()) {
        auto filter_patterns = get_filter_patterns(options, "whatsuggests");

        libdnf5::rpm::PackageQuery by_package_query(query);
        by_package_query.filter_suggests(resolve_nevras(query, filter_patterns));

        query.filter_suggests(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        query |= by_package_query;
    }
    if (options.find("whatsupplements") != options.end()) {
        auto filter_patterns = get_filter_patterns(options, "whatsupplements");

        libdnf5::rpm::PackageQuery by_package_query(query);
        by_package_query.filter_supplements(resolve_nevras(query, filter_patterns));

        query.filter_supplements(filter_patterns, libdnf5::sack::QueryCmp::GLOB);
        query |= by_package_query;
    }

    // finally apply latest filter
    if (options.find("latest-limit") != options.end()) {
        query.filter_latest_evr(key_value_map_get<int>(options, "latest-limit"));
    }

    // create reply from the query
    dnfdaemon::KeyValueMapList out_packages;
    std::vector<std::string> default_attrs{};
    std::vector<std::string> package_attrs =
        key_value_map_get<std::vector<std::string>>(options, "package_attrs", default_attrs);
    for (const auto & pkg : query) {
        out_packages.push_back(package_to_map(pkg, package_attrs));
    }

    auto reply = call.createReply();
    reply << out_packages;
    return reply;
}

sdbus::MethodReply Rpm::distro_sync(sdbus::MethodCall & call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    // fill the goal
    auto & goal = session.get_goal();
    libdnf5::GoalJobSettings setting;
    if (specs.empty()) {
        goal.add_rpm_distro_sync(setting);
    } else {
        for (const auto & spec : specs) {
            goal.add_rpm_distro_sync(spec, setting);
        }
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::downgrade(sdbus::MethodCall & call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf5::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_downgrade(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::install(sdbus::MethodCall & call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    libdnf5::GoalSetting skip_broken;
    if (options.find("skip_broken") != options.end()) {
        skip_broken = dnfdaemon::key_value_map_get<bool>(options, "skip_broken") ? libdnf5::GoalSetting::SET_TRUE
                                                                                 : libdnf5::GoalSetting::SET_FALSE;
    } else {
        skip_broken = libdnf5::GoalSetting::AUTO;
    }
    libdnf5::GoalSetting skip_unavailable;
    if (options.find("skip_unavailable") != options.end()) {
        skip_unavailable = dnfdaemon::key_value_map_get<bool>(options, "skip_unavailable")
                               ? libdnf5::GoalSetting::SET_TRUE
                               : libdnf5::GoalSetting::SET_FALSE;
    } else {
        skip_unavailable = libdnf5::GoalSetting::AUTO;
    }
    std::vector<std::string> repo_ids = dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf5::GoalJobSettings setting;
    setting.skip_broken = skip_broken;
    setting.skip_unavailable = skip_unavailable;
    setting.to_repo_ids = repo_ids;

    for (const auto & spec : specs) {
        goal.add_install(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::upgrade(sdbus::MethodCall & call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf5::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    if (specs.empty()) {
        goal.add_rpm_upgrade(setting);
    } else {
        for (const auto & spec : specs) {
            goal.add_upgrade(spec, setting);
        }
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::reinstall(sdbus::MethodCall & call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;
    std::vector<std::string> repo_ids = dnfdaemon::key_value_map_get<std::vector<std::string>>(options, "repo_ids", {});

    // fill the goal
    auto & goal = session.get_goal();
    libdnf5::GoalJobSettings setting;
    setting.to_repo_ids = repo_ids;
    for (const auto & spec : specs) {
        goal.add_reinstall(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}

sdbus::MethodReply Rpm::remove(sdbus::MethodCall & call) {
    std::vector<std::string> specs;
    call >> specs;

    // read options from dbus call
    dnfdaemon::KeyValueMap options;
    call >> options;

    // fill the goal
    auto & goal = session.get_goal();

    // Limit remove spec capabity to prevent multiple matches. Remove command should not match anything after performing
    // a remove action with the same spec. NEVRA and filenames are the only types that have no overlaps.
    libdnf5::GoalJobSettings setting;
    setting.with_nevra = true;
    setting.with_provides = false;
    setting.with_filenames = true;
    setting.with_binaries = false;
    for (const auto & spec : specs) {
        goal.add_remove(spec, setting);
    }

    auto reply = call.createReply();
    return reply;
}
