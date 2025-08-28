// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

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
