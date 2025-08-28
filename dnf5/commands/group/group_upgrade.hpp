// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_GROUP_GROUP_UPGRADE_HPP
#define DNF5_COMMANDS_GROUP_GROUP_UPGRADE_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class GroupUpgradeCommand : public Command {
public:
    explicit GroupUpgradeCommand(Context & context) : Command(context, "upgrade") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::unique_ptr<AllowErasingOption> allow_erasing;

    std::unique_ptr<GroupSpecArguments> group_specs{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_GROUP_UPGRADE_HPP
