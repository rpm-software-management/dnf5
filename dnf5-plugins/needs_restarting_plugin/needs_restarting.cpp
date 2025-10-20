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

#include <dirent.h>
#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/output/changelogs.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/sdbus_compat.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <unistd.h>
#include <utils/string.hpp>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>

const std::string SYSTEMD_DESTINATION_NAME{"org.freedesktop.systemd1"};
const sdbus::ObjectPath SYSTEMD_OBJECT_PATH{"/org/freedesktop/systemd1"};
const SDBUS_INTERFACE_NAME_TYPE SYSTEMD_MANAGER_INTERFACE{"org.freedesktop.systemd1.Manager"};
const SDBUS_INTERFACE_NAME_TYPE SYSTEMD_UNIT_INTERFACE{"org.freedesktop.systemd1.Unit"};
const SDBUS_INTERFACE_NAME_TYPE SYSTEMD_SERVICE_INTERFACE{"org.freedesktop.systemd1.Service"};

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

    processes_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));

    auto * processes_arg = parser.add_new_named_arg("processes");
    processes_arg->set_long_name("processes");
    processes_arg->set_short_name('p');
    processes_arg->set_description("List processes started before their dependencies were updated");
    processes_arg->set_const_value("true");
    processes_arg->link_value(processes_option);
    cmd.register_named_arg(processes_arg);

    exclude_services_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));

    auto * exclude_services_arg = parser.add_new_named_arg("exclude-services");
    exclude_services_arg->set_long_name("exclude-services");
    exclude_services_arg->set_short_name('e');
    exclude_services_arg->set_description("Exclude processes managed by systemd services (use with --processes)");
    exclude_services_arg->set_const_value("true");
    exclude_services_arg->link_value(exclude_services_option);
    cmd.register_named_arg(exclude_services_arg);

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

time_t NeedsRestartingCommand::get_proc_1_boot_time(Context & ctx) {
    time_t proc_1_boot_time = 0;
    struct stat proc_1_stat = {};
    const auto & logger = ctx.get_base().get_logger();

    if (stat("/proc/1", &proc_1_stat) == 0) {
        proc_1_boot_time = proc_1_stat.st_mtime;
    } else {
        logger->debug("Unable to stat /proc/1, using {} as the /proc/1 boot time.", proc_1_boot_time);
    }

    return proc_1_boot_time;
}

time_t NeedsRestartingCommand::get_kernel_boot_time(Context & ctx) {
    time_t btime = 0;
    const auto & logger = ctx.get_base().get_logger();
    std::ifstream proc_stat{"/proc/stat"};
    std::string line;

    if (!proc_stat.is_open()) {
        logger->debug("Unable to read /proc/stat, using {} as the system boot time.", btime);
        return btime;
    }

    while (std::getline(proc_stat, line)) {
        if (line.compare(0, 6, "btime ") == 0) {
            std::istringstream btime_stream(line);
            std::string tmp;
            btime_stream >> tmp >> btime;
            return btime;
        }
    }

    if (btime == 0) {
        logger->debug("Error reading /proc/stat, using {} as the system boot time.", btime);
    }

    logger->debug("Using {} as the system boot time.", btime);
    return btime;
}

time_t NeedsRestartingCommand::get_boot_time(Context & ctx) {
    // We have three sources from which to derive the boot time. These values
    // vary depending on containerization, existing of a Real Time Clock, etc:
    // - UnitsLoadStartTimestamp property on /org/freedesktop/systemd1
    //      The start time of the service manager, according to systemd itself.
    //      Seems to be more reliable than UserspaceTimestamp when the RTC is
    //      in local time. Works unless the system was not booted with systemd,
    //      such as in (most) containers.
    //      Reflects the time the first process was run after booting. This
    //      works for all known cases except machines without a RTC---they
    //      awake at the start of the epoch.
    // - st_mtime of /proc/1
    //      Reflects the time the first process was run after booting. This
    //      works for all known cases except machines without a RTC---they
    //      awake at the start of the epoch.
    // - btime field of /proc/stat
    //      Reflects the time when the kernel started. Works for machines
    //      without RTC iff the current time is reasonably correct. Does not
    //      work on containers which share their kernel with the host---there,
    //      the host kernel uptime is returned.

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

    // Otherwise, take the maximum of the st_mtime of /proc/1 and the btime field of /proc/stat.
    logger->debug(
        "Couldn't get boot time from systemd, checking st_mtime of /proc/1 and the btime field of /proc/stat.");
    time_t proc_1_boot_time = get_proc_1_boot_time(ctx);
    time_t kernel_boot_time = get_kernel_boot_time(ctx);
    const time_t boot_time = std::max(proc_1_boot_time, kernel_boot_time);

    logger->debug("st_mtime of /proc/1: {}", proc_1_boot_time);
    logger->debug("btime field of /proc/stat: {}", kernel_boot_time);
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

std::vector<NeedsRestartingCommand::SystemdService> NeedsRestartingCommand::get_systemd_services(
    [[maybe_unused]] Context & ctx) {
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

    std::vector<SystemdService> services;

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
        std::string active_state =
            unit_proxy->getProperty("ActiveState").onInterface(SYSTEMD_UNIT_INTERFACE).get<std::string>();
        if (active_state != "active") {
            continue;
        }

        // FragmentPath is the path to the unit file that defines the service
        std::string fragment_path =
            unit_proxy->getProperty("FragmentPath").onInterface(SYSTEMD_UNIT_INTERFACE).get<std::string>();
        const auto start_timestamp_us =
            uint64_t{unit_proxy->getProperty("ActiveEnterTimestamp").onInterface(SYSTEMD_UNIT_INTERFACE)};

        services.push_back(SystemdService{unit_name, start_timestamp_us, unit_object_path, fragment_path});
    }

    return services;
}

void NeedsRestartingCommand::services_need_restarting(Context & ctx) {
    const auto services = get_systemd_services(ctx);

    std::unordered_map<std::string, SystemdService> unit_file_to_service;
    for (const auto & service : services) {
        unit_file_to_service.insert(std::make_pair(service.fragment_path, service));
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

void NeedsRestartingCommand::processes_need_restarting(Context & ctx, bool exclude_services) {
    const auto boot_time = get_boot_time(ctx);
    const auto & logger = ctx.get_base().get_logger();

    // Build a map of executable paths to their packages
    libdnf5::rpm::PackageQuery installed{ctx.get_base()};
    installed.filter_installed();

    std::unordered_map<std::string, libdnf5::rpm::Package> file_to_package;
    for (const auto & package : installed) {
        for (const auto & file : package.get_files()) {
            file_to_package.insert({file, package});
        }
    }

    // If exclude_services is set, build a set of PIDs managed by systemd services
    std::unordered_set<std::string> service_pids;
    if (exclude_services) {
        try {
            std::unique_ptr<sdbus::IConnection> connection = sdbus::createSystemBusConnection();
            const auto services = get_systemd_services(ctx);

            for (const auto & service : services) {
                auto unit_proxy = sdbus::createProxy(SYSTEMD_DESTINATION_NAME, service.object_path);

                // Get the main PID of the service
                try {
                    const auto main_pid =
                        uint32_t{unit_proxy->getProperty("MainPID").onInterface(SYSTEMD_SERVICE_INTERFACE)};
                    if (main_pid > 0) {
                        service_pids.insert(std::to_string(main_pid));
                    }
                } catch (const sdbus::Error & ex) {
                    // Some services might not have a MainPID
                    logger->debug("Failed to get MainPID for {}: {}", service.name, ex.what());
                }

                // Also get control group PIDs
                try {
                    std::string control_group = unit_proxy->getProperty("ControlGroup")
                                                    .onInterface(SYSTEMD_SERVICE_INTERFACE)
                                                    .get<std::string>();
                    if (!control_group.empty()) {
                        // Read PIDs from the cgroup
                        std::string cgroup_procs_path = "/sys/fs/cgroup" + control_group + "/cgroup.procs";
                        if (std::ifstream cgroup_file(cgroup_procs_path); cgroup_file.is_open()) {
                            std::string pid;
                            while (std::getline(cgroup_file, pid)) {
                                if (!pid.empty()) {
                                    service_pids.insert(pid);
                                }
                            }
                        }
                    }
                } catch (const sdbus::Error & ex) {
                    logger->debug("Failed to get ControlGroup for {}: {}", service.name, ex.what());
                }
            }
        } catch (const sdbus::Error & ex) {
            logger->warning("Failed to connect to D-Bus for service exclusion: {}", ex.what());
        } catch (const libdnf5::cli::CommandExitError & ex) {
            logger->warning("Failed to get systemd services for exclusion: {}", ex.what());
        }
    }

    // Iterate through /proc to find running processes
    DIR * proc_dir = opendir("/proc");
    if (!proc_dir) {
        logger->warning("Failed to open /proc directory");
        return;
    }

    // Map of executable paths to their process start times and packages
    struct ProcessInfo {
        time_t start_time;
        libdnf5::rpm::Package package;
    };
    std::unordered_map<std::string, ProcessInfo> running_processes;

    struct dirent * entry;

    while ((entry = readdir(proc_dir)) != nullptr) {
        // Check if the directory name is a number (PID)
        if (entry->d_type != DT_DIR) {
            continue;
        }

        const char * name = entry->d_name;
        bool is_pid = true;
        for (const char * c = name; *c != '\0'; ++c) {
            if (!std::isdigit(*c)) {
                is_pid = false;
                break;
            }
        }

        if (!is_pid) {
            continue;
        }

        // Skip processes managed by systemd services if exclude_services is set
        if (exclude_services && service_pids.find(name) != service_pids.end()) {
            continue;
        }

        // Get the process start time from /proc/[pid]/stat
        std::string stat_path = std::string("/proc/") + name + "/stat";
        std::ifstream stat_file(stat_path);
        time_t process_start_time = 0;

        if (stat_file.is_open()) {
            std::string stat_line;
            if (std::getline(stat_file, stat_line)) {
                // Parse /proc/[pid]/stat to get the starttime field (field 22)
                // Format: pid (comm) state ppid pgrp session tty_nr tpgid flags ... starttime
                size_t pos = stat_line.rfind(')');
                if (pos != std::string::npos) {
                    std::istringstream iss(stat_line.substr(pos + 1));
                    std::string field;
                    // Skip fields 3-21 (we're after field 2 which is comm)
                    for (int i = 0; i < 20; ++i) {
                        iss >> field;
                    }
                    // Field 22 is starttime in clock ticks since boot
                    unsigned long long starttime_ticks;
                    if (iss >> starttime_ticks) {
                        // Convert ticks to seconds and add to boot_time
                        long ticks_per_second = sysconf(_SC_CLK_TCK);
                        process_start_time =
                            boot_time +
                            static_cast<time_t>(starttime_ticks / static_cast<unsigned long long>(ticks_per_second));
                    }
                }
            }
        }

        // Read the exe symlink to get the executable path
        std::string exe_link = std::string("/proc/") + name + "/exe";
        char exe_path[PATH_MAX];
        ssize_t len = readlink(exe_link.c_str(), exe_path, sizeof(exe_path) - 1);

        if (len != -1) {
            exe_path[len] = '\0';

            // Remove " (deleted)" suffix if present
            std::string exe_path_str(exe_path);
            const std::string deleted_suffix = " (deleted)";
            if (libdnf5::utils::string::ends_with(exe_path_str, deleted_suffix)) {
                exe_path_str = exe_path_str.substr(0, exe_path_str.length() - deleted_suffix.length());
            }

            // Check if this executable is from an installed package
            auto it = file_to_package.find(exe_path_str);
            if (it != file_to_package.end()) {
                const auto & package = it->second;
                running_processes.insert({exe_path_str, ProcessInfo{process_start_time, package}});
            }
        }
    }

    closedir(proc_dir);

    // Now check which processes need restarting
    std::unordered_set<std::string> processes_needing_restart;
    libdnf5::rpm::PackageSet updated_packages{ctx.get_base()};

    for (const auto & [exe_path, process_info] : running_processes) {
        // Recursively get all dependencies of the package that
        // provides the executable (and include the package itself)
        const auto & deps = recursive_dependencies(process_info.package, installed);
        for (const auto & dep : deps) {
            // If any dependency (or the package itself) has been
            // updated since the process started, the process needs restart
            const auto install_time = static_cast<time_t>(dep.get_install_time());
            if (install_time > process_info.start_time) {
                processes_needing_restart.insert(exe_path);
                // Track this package as updated so we can check reverse dependencies
                updated_packages.add(dep);
                break;
            }
        }
    }

    if (!processes_needing_restart.empty()) {
        std::vector<std::string> sorted_processes(processes_needing_restart.begin(), processes_needing_restart.end());
        std::sort(sorted_processes.begin(), sorted_processes.end());
        for (const auto & process : sorted_processes) {
            std::cout << process << std::endl;
        }
        throw libdnf5::cli::SilentCommandExitError(1);
    }
}

void NeedsRestartingCommand::run() {
    auto & ctx = get_context();

    if (services_option->get_value()) {
        services_need_restarting(ctx);
    } else if (processes_option->get_value()) {
        processes_need_restarting(ctx, exclude_services_option->get_value());
    } else {
        system_needs_restarting(ctx);
    }
}

}  // namespace dnf5
