// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_CONFIG_MANAGER_UNSETVAR_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_UNSETVAR_HPP

#include <dnf5/context.hpp>

#include <set>
#include <string>

namespace dnf5 {

class ConfigManagerUnsetVarCommand : public Command {
public:
    explicit ConfigManagerUnsetVarCommand(Context & context) : Command(context, "unsetvar") {}
    void set_argument_parser() override;
    void configure() override;

private:
    std::set<std::string> vars_to_remove;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_UNSETVAR_HPP
