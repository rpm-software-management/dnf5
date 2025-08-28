// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_REMOVE_REMOVE_HPP
#define DNF5_COMMANDS_REMOVE_REMOVE_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

class RemoveCommand : public Command {
public:
    explicit RemoveCommand(Context & context) : Command(context, "remove") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs;
    std::vector<std::string> installed_from_repos;
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_REMOVE_REMOVE_HPP
