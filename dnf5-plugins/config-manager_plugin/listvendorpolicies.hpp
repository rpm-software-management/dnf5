// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_CONFIG_MANAGER_LISTVENDORPOLICIES_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_LISTVENDORPOLICIES_HPP

#include <dnf5/context.hpp>

#include <filesystem>
#include <string>

namespace dnf5 {

class ConfigManagerListVendorPoliciesCommand final : public Command {
public:
    explicit ConfigManagerListVendorPoliciesCommand(Context & context) : Command(context, "list-vendor-policies") {}
    void set_argument_parser() override;
    void configure() override;

private:
    void print_policy_info(const std::filesystem::path & policy_file, const std::filesystem::path & mask_file);

    std::string name_pattern;  // Glob patterns for policy filenames (optional)
    bool info{false};          // Show detailed information
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_LISTVENDORPOLICIES_HPP
