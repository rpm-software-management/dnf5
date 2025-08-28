// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_GROUP_GROUP_REMOVE_HPP
#define DNF5_COMMANDS_GROUP_GROUP_REMOVE_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class GroupRemoveCommand : public Command {
public:
    explicit GroupRemoveCommand(Context & context) : GroupRemoveCommand(context, "remove") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::unique_ptr<GroupNoPackagesOption> no_packages{nullptr};
    std::unique_ptr<GroupSpecArguments> group_specs{nullptr};

protected:
    // to be used by an alias command only
    explicit GroupRemoveCommand(Context & context, const std::string & name) : Command(context, name) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_GROUP_REMOVE_HPP
