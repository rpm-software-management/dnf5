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

#ifndef DNF5_COMMANDS_NEEDS_RESTARTING_HPP
#define DNF5_COMMANDS_NEEDS_RESTARTING_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <sys/stat.h>

#include <exception>
#include <memory>


namespace dnf5 {

class NeedsRestartingCommand : public Command {
public:
    explicit NeedsRestartingCommand(Context & context) : Command(context, "needs-restarting") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    struct SystemdService {
        std::string name;
        uint64_t start_timestamp_us;
        sdbus::ObjectPath object_path;
        std::string fragment_path;
    };
    sdbus::IConnection & get_system_bus_connection();
    std::unique_ptr<sdbus::IConnection> system_bus_connection{nullptr};
    std::exception_ptr system_bus_connection_exception;
    libdnf5::OptionBool * services_option{nullptr};
    libdnf5::OptionBool * processes_option{nullptr};
    libdnf5::OptionBool * exclude_services_option{nullptr};
    static time_t get_proc_1_boot_time(Context &);
    static time_t get_kernel_boot_time(Context &);
    time_t get_boot_time(Context &);
    void system_needs_restarting(Context &);
    std::vector<SystemdService> get_systemd_services(Context &);
    void services_need_restarting(Context &);
    void processes_need_restarting(Context &, bool exclude_services);
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_NEEDS_RESTARTING_HPP
