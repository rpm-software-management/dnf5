// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_LEAVES_LEAVES_HPP
#define DNF5_COMMANDS_LEAVES_LEAVES_HPP

#include <dnf5/context.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

class LeavesCommand : public Command {
public:
    explicit LeavesCommand(Context & context) : Command(context, "leaves") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_LEAVES_LEAVES_HPP
