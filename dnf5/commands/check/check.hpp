// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_CHECK_CHECK_HPP
#define DNF5_COMMANDS_CHECK_CHECK_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class CheckCommand : public Command {
public:
    explicit CheckCommand(Context & context) : Command(context, "check") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_CHECK_CHECK_HPP
