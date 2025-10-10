// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP
#define LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP

#include "libdnf5/common/sack/query_cmp.hpp"

#include <bitset>
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
    static constexpr unsigned int MAX_VENDOR_POLICIES = 64;

    struct VendorChangePolicy {
        struct VendorDef {
            std::string vendor;
            sack::QueryCmp comparator;
            bool is_exclusion;
        };
        std::vector<VendorDef> outgoing_vendors;
        std::vector<VendorDef> incoming_vendors;
    };

    struct VendorChangeMasks {
        Id vendor;
        std::bitset<MAX_VENDOR_POLICIES> outgoing_mask;
        std::bitset<MAX_VENDOR_POLICIES> incoming_mask;
    };

    VendorChangeManager(const Pool & pool);
    const VendorChangeMasks & get_vendor_change_masks(Id vendor);

    void load_vendor_change_policy(const std::filesystem::path & path);

private:
    const Pool & pool;
    std::vector<VendorChangePolicy> vendor_policies_def;
    std::vector<VendorChangeMasks> vendor_masks;
};

}  // namespace libdnf5::solv

#endif  // LIBDNF5_SOLV_VENDOR_CHANGE_MANAGER_HPP
