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

#ifndef DNF5_COMMANDS_NEEDS_RESTARTING_HPP
#define DNF5_COMMANDS_NEEDS_RESTARTING_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <sys/stat.h>


namespace dnf5 {

class NeedsRestartingCommand : public Command {
public:
    explicit NeedsRestartingCommand(Context & context) : Command(context, "needs-restarting") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    libdnf5::OptionBool * services_option{nullptr};
    static time_t get_boot_time(Context &);
    static void system_needs_restarting(Context &);
    static void services_need_restarting(Context &);
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_NEEDS_RESTARTING_HPP
