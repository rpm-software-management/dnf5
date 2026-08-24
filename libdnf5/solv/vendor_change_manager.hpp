// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP
#define LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP

#include "solv_map.hpp"

#include "libdnf5/common/sack/query_cmp.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

extern "C" {
#include <solv/pooltypes.h>
}

namespace libdnf5::solv {

class Pool;

/// Internal vendor change policy engine integrated with the libsolv pool.
///
/// Holds the list of vendor change policies, evaluates whether a vendor transition
/// between two solvables is allowed, and caches per-vendor bitmasks for fast lookups.
///
/// Policies are loaded from TOML files or compact strings and stored as
/// VendorChangePolicy structs. During solver operation, libsolv calls
/// `is_vendor_change_allowed()` via a callback registered by RpmPool.
class VendorChangeManager {
public:
    /// Represents a single vendor change policy — the runtime equivalent of one
    /// TOML configuration file or one compact policy string.
    struct VendorChangePolicy {
        struct PackageDef {
            struct Filter;

            /// Function pointer type for package filtering logic
            /// @return true if the solvable matches the filter criteria
            using FilterFunction = bool (*)(const Pool &, const Solvable &, const Filter &);

            struct Filter {
                FilterFunction filter_func;  // The logic used to evaluate the solvable
                std::string value;           // The value or pattern to compare against
                sack::QueryCmp comparator;   // Comparison operator (e.g., EXACT)
            };
            std::vector<Filter> filters;  // List of filters that must all match
            bool is_exclusion;            // Whether matching packages are excluded from the policy
        };

        struct VendorDef {
            std::string vendor;         // Vendor name or pattern
            sack::QueryCmp comparator;  // Comparison operator for the vendor name
            bool is_exclusion;          // Whether this vendor is excluded from the policy
        };

        enum class VendorGroupType { OUTGOING, INCOMING, EQUIVALENT };

        struct VendorEntry {
            VendorGroupType group_type;
            VendorDef def;
        };

        std::vector<PackageDef> outgoing_packages;
        std::vector<PackageDef> incoming_packages;
        std::vector<VendorEntry> vendor_entries;  // Ordered list preserving original group types
        std::string source;                       // Origin of the policy (file URI or custom label with "text:" prefix)
    };

    /// Constructor
    /// @param pool Reference to the solvable pool
    VendorChangeManager(const Pool & pool);

    /// Parse a vendor change policy from TOML content and add it.
    /// @param toml_content The TOML content to parse
    /// @param source Origin of the policy (file URI or custom label with "text:" prefix)
    void add_policy_from_toml(std::string_view toml_content, std::string_view source);

    /// Load one vendor change policy from a TOML configuration file and add it.
    /// @param path Path to the configuration file
    void add_policy_from_toml(const std::filesystem::path & path);

    /// Parse a vendor change policy from a compact representation and add it.
    /// Format: direction:eop"value",...@direction:e[filters],...
    /// @param policy_str The policy string to parse
    /// @param source Origin of the policy (file URI or custom label with "text:" prefix)
    void add_policy_from_compact(std::string_view policy_str, std::string_view source);

    /// Remove all vendor change policies.
    void clear_policies();

    /// Remove a vendor change policy at the specified index.
    /// @param index The index of the policy to remove (0-based)
    /// @throws base::VendorChangeManagerError if index is out of bounds
    void remove_policy(std::size_t index);

    /// Remove vendor change policies whose source matches the specified glob pattern.
    /// @param source_pattern A glob pattern to match against policy sources
    /// @return The number of policies that were removed
    std::size_t remove_policies_matching_source(const std::string & source_pattern);

    /// Get the number of registered vendor change policies.
    /// @return The number of policies in vendor_policies_def
    [[nodiscard]] std::size_t get_policies_count() const noexcept { return vendor_policies_def.size(); }

    /// Get the source (origin) of a vendor change policy.
    /// @param index The index of the policy in vendor_policies_def
    /// @return The source string (file URI or custom label with "text:" prefix)
    /// @throws base::VendorChangeManagerError if index is out of bounds
    [[nodiscard]] const std::string & get_policy_source(std::size_t index) const;

    /// Get a vendor change policy as a TOML string.
    /// @param index The index of the policy in vendor_policies_def
    /// @return The policy formatted as a TOML string
    /// @throws base::VendorChangeManagerError if index is out of bounds
    [[nodiscard]] std::string get_policy_as_toml(std::size_t index) const;

    /// Get a vendor change policy as a compact format string.
    /// @param index The index of the policy in vendor_policies_def
    /// @return The policy formatted as a compact string
    /// @throws base::VendorChangeManagerError if index is out of bounds
    [[nodiscard]] std::string get_policy_as_compact(std::size_t index) const;

    /// Convert a vendor change policy from TOML content to compact format.
    /// @param toml_content The TOML content to parse
    /// @param source Origin of the policy (for error messages)
    /// @return The policy formatted as a compact string
    /// @throws base::VendorChangeManagerError if parsing fails
    static std::string convert_policy_toml_to_compact(std::string_view toml_content, std::string_view source);

    /// Convert a vendor change policy from TOML file to compact format.
    /// @param path Path to the TOML configuration file
    /// @return The policy formatted as a compact string
    /// @throws base::VendorChangeManagerError if parsing fails
    static std::string convert_policy_toml_to_compact(const std::filesystem::path & path);

    /// Convert a vendor change policy from compact format to TOML format.
    /// @param compact_str The policy in compact format
    /// @param source Origin of the policy (for error messages)
    /// @return The policy formatted as a TOML string
    /// @throws base::VendorChangeManagerError if parsing fails
    static std::string convert_policy_compact_to_toml(std::string_view compact_str, std::string_view source);

    /// Check if a vendor change is allowed between two solvables
    /// @param outgoing The currently installed solvable
    /// @param incoming The candidate solvable for replacement
    /// @return true if the transition is permitted by the policies
    [[nodiscard]] bool is_vendor_change_allowed(Solvable & outgoing, Solvable & incoming);

    /// Check if a solvable is in the list of incoming solvables that bypass vendor check.
    /// @param solvable_id The solvable ID to check
    /// @return true if the solvable bypasses vendor check, false otherwise
    [[nodiscard]] bool is_incoming_vendor_bypassed_solvable(Id solvable_id) const noexcept {
        return incoming_vendor_bypassed_solvables.contains(solvable_id);
    }

    /// Get a reference to the map of incoming solvables that bypass vendor check.
    /// This allows direct manipulation of the solvables map.
    /// @return Reference to the SolvMap containing solvable IDs that bypass vendor check
    [[nodiscard]] SolvMap & get_incoming_vendor_bypassed_solvables() noexcept {
        return incoming_vendor_bypassed_solvables;
    }

    /// Get a const reference to the map of incoming solvables that bypass vendor check.
    /// @return Const reference to the SolvMap containing solvable IDs that bypass vendor check
    [[nodiscard]] const SolvMap & get_incoming_vendor_bypassed_solvables() const noexcept {
        return incoming_vendor_bypassed_solvables;
    }

private:
    struct VendorChangeMasks {
        SolvMap outgoing_mask{0};
        SolvMap incoming_mask{0};
    };

    void add_policy(VendorChangePolicy && policy);

    const Pool & pool;
    std::vector<VendorChangePolicy> vendor_policies_def;
    std::map<Id, VendorChangeMasks> vendor_masks;
    SolvMap incoming_vendor_bypassed_solvables{0};

    /// Retrieve or cache masks for a specific vendor
    /// @param vendor The vendor ID
    /// @return Reference to the calculated VendorChangeMasks
    const VendorChangeMasks & get_vendor_change_masks(Id vendor);

    /// Check if a solvable matches any of the provided package definitions
    /// @param pkgs_def List of package definitions (including exclusions)
    /// @param solvable The solvable to evaluate
    /// @return true if the solvable matches the criteria and is not excluded
    bool matches_package_defs(const std::vector<VendorChangePolicy::PackageDef> & pkgs_def, const Solvable & solvable);

    /// Convert path to source string
    /// @param path Path to convert
    /// @return Source string (file URI)
    static std::string path_to_source(const std::filesystem::path & path);
};

}  // namespace libdnf5::solv

#endif  // LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP
