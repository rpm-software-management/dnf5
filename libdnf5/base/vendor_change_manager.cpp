// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/vendor_change_manager.hpp"

#include "base_impl.hpp"
#include "solv/pool.hpp"
#include "solv/vendor_change_manager.hpp"

namespace libdnf5::base {

class VendorChangeManager::Impl {
public:
    explicit Impl(solv::RpmPool & pool) : vcm(pool.get_vendor_change_manager()) {}

    void add_policy_from_toml(const std::filesystem::path & path) { vcm.add_policy_from_toml(path); }

    void add_policy_from_compact(std::string_view policy_str, std::string_view source) {
        vcm.add_policy_from_compact(policy_str, source);
    }

    void clear_policies() { vcm.clear_policies(); }

    WeakPtrGuard<VendorChangeManager, false> guard;

private:
    solv::VendorChangeManager & vcm;
};


VendorChangeManager::VendorChangeManager(Base & base) : p_impl(new Impl(get_rpm_pool(base.get_weak_ptr()))) {}

VendorChangeManager::~VendorChangeManager() = default;

VendorChangeManagerWeakPtr VendorChangeManager::get_weak_ptr() {
    return {this, &p_impl->guard};
}

void VendorChangeManager::add_policy_from_toml(const std::filesystem::path & path) {
    p_impl->add_policy_from_toml(path);
}

void VendorChangeManager::add_policy_from_compact(std::string_view policy_str, std::string_view source) {
    p_impl->add_policy_from_compact(policy_str, source);
}

void VendorChangeManager::clear_policies() {
    p_impl->clear_policies();
}

}  // namespace libdnf5::base
