// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP
#define LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP

#include "solv_map.hpp"

#include "libdnf5/common/sack/query_cmp.hpp"

#include <filesystem>
#include <string>
#include <vector>

extern "C" {
#include <solv/pooltypes.h>
}

namespace libdnf5::solv {

class Pool;

class VendorChangeManager {
public:
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
            sack::QueryCmp comparator;  //Comparison operator for the vendor name
            bool is_exclusion;          // Whether this vendor is excluded from the policy
        };

        std::vector<PackageDef> outgoing_packages;
        std::vector<PackageDef> incoming_packages;
        std::vector<VendorDef> outgoing_vendors;
        std::vector<VendorDef> incoming_vendors;
    };

    /// Constructor
    /// @param pool Reference to the solvable pool
    VendorChangeManager(const Pool & pool);

    /// Load one vendor change policy from configuration file
    /// @param path Path to the configuration file
    void load_vendor_change_policy(const std::filesystem::path & path);

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
        Id vendor;
        SolvMap outgoing_mask{0};
        SolvMap incoming_mask{0};
    };

    const Pool & pool;
    std::vector<VendorChangePolicy> vendor_policies_def;
    std::vector<VendorChangeMasks> vendor_masks;
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
};

}  // namespace libdnf5::solv

#endif  // LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP
