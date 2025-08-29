// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_CONFIG_MANAGER_SETVAR_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_SETVAR_HPP

#include <dnf5/context.hpp>

#include <map>
#include <string>

namespace dnf5 {

class ConfigManagerSetVarCommand : public Command {
public:
    explicit ConfigManagerSetVarCommand(Context & context) : Command(context, "setvar") {}
    void set_argument_parser() override;
    void configure() override;

private:
    std::map<std::string, std::string> setvars;
    bool create_missing_dirs{false};  // Allows one to create missing directories.
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_SETVAR_HPP
