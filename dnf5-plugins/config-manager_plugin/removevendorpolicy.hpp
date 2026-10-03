// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_CONFIG_MANAGER_REMOVEVENDORPOLICY_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_REMOVEVENDORPOLICY_HPP

#include <dnf5/context.hpp>

#include <string>

namespace dnf5 {

class ConfigManagerRemoveVendorPolicyCommand final : public Command {
public:
    explicit ConfigManagerRemoveVendorPolicyCommand(Context & context) : Command(context, "remove-vendor-policy") {}
    void set_argument_parser() override;
    void configure() override;

private:
    std::string name_pattern;  // Glob pattern for policy names
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_REMOVEVENDORPOLICY_HPP
