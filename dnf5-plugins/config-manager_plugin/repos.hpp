// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_COMMANDS_CONFIG_MANAGER_REPOS_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_REPOS_HPP

#include "shared.hpp"

#include <dnf5/context.hpp>

#include <set>
#include <string>

namespace dnf5 {

using namespace libdnf5;

class RepoCommand : public Command {
public:
    explicit RepoCommand(Context & context, const std::string & name, const std::string & value)
        : Command(context, name),
          enabled_value(value) {}

    void set_argument_parser() override;
    void configure() override;

protected:
    virtual std::string get_command_description() const = 0;

private:
    const std::string enabled_value;

    std::set<std::string> repos;
    bool create_missing_dirs{false};
};

class ConfigManagerEnableCommand : public RepoCommand {
public:
    explicit ConfigManagerEnableCommand(Context & context) : RepoCommand(context, "enable", "1") {}

protected:
    std::string get_command_description() const override { return _("Enables repositories"); }
};

class ConfigManagerDisableCommand : public RepoCommand {
public:
    explicit ConfigManagerDisableCommand(Context & context) : RepoCommand(context, "disable", "0") {}

protected:
    std::string get_command_description() const override { return _("Disables repositories"); }
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_REPOS_HPP
