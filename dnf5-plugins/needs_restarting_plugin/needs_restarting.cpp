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

#include "needs_restarting.hpp"

#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/output/changelogs.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/sdbus_compat.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <utils/string.hpp>

#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

const SDBUS_SERVICE_NAME_TYPE SYSTEMD_DESTINATION_NAME{"org.freedesktop.systemd1"};
const sdbus::ObjectPath SYSTEMD_OBJECT_PATH{"/org/freedesktop/systemd1"};
const SDBUS_INTERFACE_NAME_TYPE SYSTEMD_MANAGER_INTERFACE{"org.freedesktop.systemd1.Manager"};
const SDBUS_INTERFACE_NAME_TYPE SYSTEMD_UNIT_INTERFACE{"org.freedesktop.systemd1.Unit"};

namespace dnf5 {

using namespace libdnf5::cli;

void NeedsRestartingCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void NeedsRestartingCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Determine whether system or systemd services need restarting");

    services_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));

    auto * services_arg = parser.add_new_named_arg("services");
    services_arg->set_long_name("services");
    services_arg->set_short_name('s');
    services_arg->set_description("List systemd services started before their dependencies were updated");
    services_arg->set_const_value("true");
    services_arg->link_value(services_option);
    cmd.register_named_arg(services_arg);

    auto * reboothint_arg = parser.add_new_named_arg("reboothint");
    reboothint_arg->set_long_name("reboothint");
    reboothint_arg->set_short_name('r');
    reboothint_arg->set_description(
        "Has no effect, kept for compatibility with DNF 4. \"dnf4 needs-restarting -r\" provides the same "
        "functionality "
        "as \"dnf5 needs-restarting\".");
    cmd.register_named_arg(reboothint_arg);
}

void NeedsRestartingCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);

    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    const std::set<std::string> metadata_types{libdnf5::METADATA_TYPE_FILELISTS, libdnf5::METADATA_TYPE_UPDATEINFO};
    context.get_base().get_config().get_optional_metadata_types_option().add(
        libdnf5::Option::Priority::RUNTIME, metadata_types);
}

time_t NeedsRestartingCommand::get_boot_time(Context & ctx) {
    // We have three sources from which to derive the boot time. These values
    // vary depending on containerization, existing of a Real Time Clock, etc:
    // - UnitsLoadStartTimestamp property on /org/freedesktop/systemd1
    //      The start time of the service manager, according to systemd itself.
    //      Seems to be more reliable than UserspaceTimestamp when the RTC is
    //      in local time. Works unless the system was not booted with systemd,
    //      such as in (most) containers.
    // - st_mtime of /proc/1
    //      Reflects the time the first process was run after booting. This
    //      works for all known cases except (1) machines without a RTC---they
    //      awake at the start of the epoch, and (2) machines with the RTC in
    //      local time (see `timedatectl status`). This method will be correct
    //      in container environments, even those running on hosts without an
    //      RTC, since the correct time should be known by the time the
    //      container is started.
    // - /proc/uptime
    //      Seconds field of /proc/uptime subtracted from the current time.
    //      Works for machines without RTC iff the current time is reasonably
    //      correct. Does not work on containers which share their kernel with
    //      the host---there, the host kernel uptime is returned

    const auto & logger = ctx.get_base().get_logger();

    // First, ask systemd for the boot time. If systemd is available, this is
    // the best option.
    try {
        std::unique_ptr<sdbus::IConnection> connection;
        connection = sdbus::createSystemBusConnection();
        auto proxy = sdbus::createProxy(SYSTEMD_DESTINATION_NAME, SYSTEMD_OBJECT_PATH);

        const auto systemd_boot_time_us =
            uint64_t{proxy->getProperty("UnitsLoadStartTimestamp").onInterface(SYSTEMD_MANAGER_INTERFACE)};

        const time_t systemd_boot_time = static_cast<long>(systemd_boot_time_us) / (1000L * 1000L);

        if (systemd_boot_time != 0) {
            logger->debug("Got boot time from systemd: {}", systemd_boot_time);
            return systemd_boot_time;
        }
    } catch (const sdbus::Error & ex) {
        // Some D-Bus error, maybe we're inside a container.
        logger->debug("D-Bus error getting boot time from systemd: {}", ex.what());
    }

    // Otherwise, take the maximum of the st_mtime of /proc/1 and /proc/uptime.
    logger->debug("Couldn't get boot time from systemd, checking st_mtime of /proc/1 and /proc/uptime.");

    time_t proc_1_boot_time = 0;
    struct stat proc_1_stat = {};
    if (stat("/proc/1", &proc_1_stat) == 0) {
        proc_1_boot_time = proc_1_stat.st_mtime;
    }

    time_t uptime_boot_time = 0;
    std::ifstream uptime_stream{"/proc/uptime"};
    if (uptime_stream.is_open()) {
        double uptime = 0;
        uptime_stream >> uptime;
        if (uptime > 0) {
            uptime_boot_time = std::time(nullptr) - static_cast<time_t>(uptime);
        }
    }

    const time_t boot_time = std::max(proc_1_boot_time, uptime_boot_time);

    logger->debug("st_mtime of /proc/1: {}", proc_1_boot_time);
    logger->debug("Boot time derived from /proc/uptime: {}", uptime_boot_time);
    logger->debug("Using {} as the system boot time.", boot_time);
    return boot_time;
}

libdnf5::rpm::PackageSet recursive_dependencies(
    const libdnf5::rpm::Package & package, libdnf5::rpm::PackageQuery & installed) {
    libdnf5::rpm::PackageSet dependencies{package.get_base()};
    dependencies.add(package);

    std::vector<libdnf5::rpm::Package> stack;
    stack.emplace_back(package);

    while (!stack.empty()) {
        libdnf5::rpm::PackageQuery query{installed};
        query.filter_provides(stack.back().get_requires());
        stack.pop_back();
        for (const auto & dependency : query) {
            if (!dependencies.contains(dependency)) {
                stack.emplace_back(dependency);
            }
        }

        dependencies |= query;
    }

    return dependencies;
}

void NeedsRestartingCommand::system_needs_restarting(Context & ctx) {
    const auto boot_time = get_boot_time(ctx);

    libdnf5::rpm::PackageQuery reboot_suggested{ctx.get_base()};
    reboot_suggested.filter_installed();
    reboot_suggested.filter_reboot_suggested();

    std::vector<libdnf5::rpm::Package> need_reboot = {};
    for (const auto & pkg : reboot_suggested) {
        if (pkg.get_install_time() > static_cast<unsigned long long>(boot_time)) {
            need_reboot.push_back(pkg);
        }
    }

    if (need_reboot.empty()) {
        std::cout << "No core libraries or services have been updated since boot-up." << std::endl
                  << "Reboot should not be necessary." << std::endl;
    } else {
        std::cout << "Core libraries or services have been updated since boot-up:" << std::endl;
        std::vector<std::string> need_reboot_names;
        for (const auto & pkg : need_reboot) {
            need_reboot_names.emplace_back(pkg.get_name());
        }
        std::sort(need_reboot_names.begin(), need_reboot_names.end());
        need_reboot_names.erase(
            std::unique(need_reboot_names.begin(), need_reboot_names.end()), need_reboot_names.end());

        for (const auto & pkg_name : need_reboot_names) {
            std::cout << "  * " << pkg_name << std::endl;
        }
        std::cout << std::endl
                  << "Reboot is required to fully utilize these updates." << std::endl
                  << "More information: https://access.redhat.com/solutions/27943" << std::endl;
        throw libdnf5::cli::SilentCommandExitError(1);
    }
}

void NeedsRestartingCommand::services_need_restarting(Context & ctx) {
    std::unique_ptr<sdbus::IConnection> connection;
    try {
        connection = sdbus::createSystemBusConnection();
    } catch (const sdbus::Error & ex) {
        const std::string error_message{ex.what()};
        throw libdnf5::cli::CommandExitError(1, M_("Couldn't connect to D-Bus: {}"), error_message);
    }

    auto systemd_proxy = sdbus::createProxy(SYSTEMD_DESTINATION_NAME, SYSTEMD_OBJECT_PATH);

    std::vector<sdbus::Struct<
        std::string,
        std::string,
        std::string,
        std::string,
        std::string,
        std::string,
        sdbus::ObjectPath,
        uint32_t,
        std::string,
        sdbus::ObjectPath>>
        units;
    systemd_proxy->callMethod("ListUnits").onInterface(SYSTEMD_MANAGER_INTERFACE).storeResultsTo(units);

    struct Service {
        std::string name;
        uint64_t start_timestamp_us;
    };

    std::unordered_map<std::string, Service> unit_file_to_service;

    for (const auto & unit : units) {
        // See ListUnits here:
        // https://www.freedesktop.org/wiki/Software/systemd/dbus/
        const auto unit_name = std::get<0>(unit);

        // Only consider service units. Skip timers, targets, etc.
        if (!libdnf5::utils::string::ends_with(unit_name, ".service")) {
            continue;
        }

        const auto unit_object_path = std::get<6>(unit);
        auto unit_proxy = sdbus::createProxy(SYSTEMD_DESTINATION_NAME, unit_object_path);

        // Only consider active (running) services
        const auto active_state = unit_proxy->getProperty("ActiveState").onInterface(SYSTEMD_UNIT_INTERFACE);
        if (std::string{active_state} != "active") {
            continue;
        }

        // FragmentPath is the path to the unit file that defines the service
        const auto fragment_path = unit_proxy->getProperty("FragmentPath").onInterface(SYSTEMD_UNIT_INTERFACE);
        const auto start_timestamp_us =
            uint64_t{unit_proxy->getProperty("ActiveEnterTimestamp").onInterface(SYSTEMD_UNIT_INTERFACE)};

        unit_file_to_service.insert(std::make_pair(fragment_path, Service{unit_name, start_timestamp_us}));
    }

    // Iterate over each file from each installed package and check whether it
    // is a unit file for a running service. This is much faster than running
    // filter_file on each unit file.
    libdnf5::rpm::PackageQuery installed{ctx.get_base()};
    installed.filter_installed();

    std::vector<std::string> service_names;
    for (const auto & package : installed) {
        for (const auto & file : package.get_files()) {
            const auto & service_pair = unit_file_to_service.find(file);
            if (service_pair != unit_file_to_service.end()) {
                // If the file is a unit file for a running service
                const auto & service = service_pair->second;

                // Recursively get all dependencies of the package that
                // provides the service (and include the package itself)
                const auto & deps = recursive_dependencies(package, installed);
                for (const auto & dep : deps) {
                    // If any dependency (or the package itself) has been
                    // updated since the service started, recommend restarting
                    // of that service
                    const uint64_t install_timestamp_us = 1000L * 1000L * dep.get_install_time();
                    if (install_timestamp_us > service.start_timestamp_us) {
                        service_names.emplace_back(service.name);
                        break;
                    }
                }
                break;
            }
        }
    }

    if (!service_names.empty()) {
        for (const auto & service_name : service_names) {
            std::cout << service_name << std::endl;
        }
        throw libdnf5::cli::SilentCommandExitError(1);
    }
}

void NeedsRestartingCommand::run() {
    auto & ctx = get_context();

    if (services_option->get_value()) {
        services_need_restarting(ctx);
    } else {
        system_needs_restarting(ctx);
    }
}

}  // namespace dnf5
