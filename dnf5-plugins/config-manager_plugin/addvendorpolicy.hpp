// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_CONFIG_MANAGER_ADDVENDORPOLICY_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_ADDVENDORPOLICY_HPP

#include <dnf5/context.hpp>

#include <optional>
#include <string>

namespace dnf5 {

class ConfigManagerAddVendorPolicyCommand final : public Command {
public:
    explicit ConfigManagerAddVendorPolicyCommand(Context & context) : Command(context, "add-vendor-policy") {}
    void set_argument_parser() override;
    void configure() override;

private:
    std::string policy_name;                    // Base filename for the policy (without .conf extension)
    std::string source_location;                // URL or local path to source TOML file
    std::optional<std::string> compact_policy;  // Compact policy string
    bool is_local_path{false};                  // Whether source_location is a local path
    bool allow_mask{false};                     // Allow masking a distribution policy from /usr/share/
    bool allow_replace{false};                  // Allow replacing an existing policy file in /etc/
    bool mask{false};                           // Require that the new policy masks a distribution policy
    bool replace{false};                        // Require that the new policy replaces an existing policy file in /etc/
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_ADDVENDORPOLICY_HPP
