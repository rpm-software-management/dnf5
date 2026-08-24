// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/vendor_change_manager.hpp"

#include "base_impl.hpp"
#include "conf/config.h"
#include "solv/pool.hpp"
#include "solv/vendor_change_manager.hpp"
#include "utils/fs/utils.hpp"

#include "libdnf5/conf/const.hpp"

namespace libdnf5::base {

class VendorChangeManager::Impl {
public:
    explicit Impl(Base & base) : base{base}, vcm{get_rpm_pool(base.get_weak_ptr()).get_vendor_change_manager()} {}

    void load_policy_from_toml(std::string_view toml_content, std::string_view source) {
        vcm.add_policy_from_toml(toml_content, source);
    }

    void load_policy_from_toml(const std::filesystem::path & path) { vcm.add_policy_from_toml(path); }

    void load_policy_from_compact(std::string_view policy_str, std::string_view source) {
        vcm.add_policy_from_compact(policy_str, source);
    }

    std::size_t unload_policies() {
        auto count = vcm.get_policies_count();
        vcm.clear_policies();
        return count;
    }

    void unload_policy(std::size_t index) { vcm.remove_policy(index); }

    std::size_t unload_policies_matching_source(const std::string & source_pattern) {
        return vcm.remove_policies_matching_source(source_pattern);
    }

    void load_policies();

    std::size_t get_loaded_policies_count() const noexcept { return vcm.get_policies_count(); }

    const std::string & get_loaded_policy_source(std::size_t index) const { return vcm.get_policy_source(index); }

    std::string get_loaded_policy_as_toml(std::size_t index) const { return vcm.get_policy_as_toml(index); }

    std::string get_loaded_policy_as_compact(std::size_t index) const { return vcm.get_policy_as_compact(index); }

    WeakPtrGuard<VendorChangeManager, false> guard;

private:
    Base & base;
    solv::VendorChangeManager & vcm;
};


void VendorChangeManager::Impl::load_policies() {
    namespace fs = std::filesystem;

    fs::path vendor_conf_dir_path{VENDOR_CONF_DIR};
    fs::path distribution_vendor_conf_dir_path{LIBDNF5_DISTRIBUTION_VENDOR_CONF_DIR};
    const bool use_installroot_config{!base.get_config().get_use_host_config_option().get_value()};
    if (use_installroot_config) {
        fs::path installroot_path{base.get_config().get_installroot_option().get_value()};
        vendor_conf_dir_path = installroot_path / vendor_conf_dir_path.relative_path();
        distribution_vendor_conf_dir_path = installroot_path / distribution_vendor_conf_dir_path.relative_path();
    }

    const auto paths =
        utils::fs::create_sorted_file_list({vendor_conf_dir_path, distribution_vendor_conf_dir_path}, ".conf");
    for (const auto & path : paths) {
        vcm.add_policy_from_toml(path);
    }
}


VendorChangeManager::VendorChangeManager(Base & base) : p_impl{new Impl(base)} {}


VendorChangeManager::~VendorChangeManager() = default;


VendorChangeManagerWeakPtr VendorChangeManager::get_weak_ptr() {
    return {this, &p_impl->guard};
}


void VendorChangeManager::load_policy_from_toml(std::string_view toml_content, std::string_view source) {
    p_impl->load_policy_from_toml(toml_content, source);
}


void VendorChangeManager::load_policy_from_toml(const std::filesystem::path & path) {
    p_impl->load_policy_from_toml(path);
}


void VendorChangeManager::load_policy_from_compact(std::string_view policy_str, std::string_view source) {
    p_impl->load_policy_from_compact(policy_str, source);
}


std::size_t VendorChangeManager::unload_policies() {
    return p_impl->unload_policies();
}


void VendorChangeManager::unload_policy(std::size_t index) {
    p_impl->unload_policy(index);
}


std::size_t VendorChangeManager::unload_policies_matching_source(const std::string & source_pattern) {
    return p_impl->unload_policies_matching_source(source_pattern);
}


std::size_t VendorChangeManager::get_loaded_policies_count() const noexcept {
    return p_impl->get_loaded_policies_count();
}


const std::string & VendorChangeManager::get_loaded_policy_source(std::size_t index) const {
    return p_impl->get_loaded_policy_source(index);
}


std::string VendorChangeManager::get_loaded_policy_as_toml(std::size_t index) const {
    return p_impl->get_loaded_policy_as_toml(index);
}


std::string VendorChangeManager::get_loaded_policy_as_compact(std::size_t index) const {
    return p_impl->get_loaded_policy_as_compact(index);
}


std::string VendorChangeManager::convert_policy_toml_to_compact(
    std::string_view toml_content, std::string_view source) {
    return solv::VendorChangeManager::convert_policy_toml_to_compact(toml_content, source);
}


std::string VendorChangeManager::convert_policy_toml_to_compact(const std::filesystem::path & path) {
    return solv::VendorChangeManager::convert_policy_toml_to_compact(path);
}


std::string VendorChangeManager::convert_policy_compact_to_toml(std::string_view compact_str, std::string_view source) {
    return solv::VendorChangeManager::convert_policy_compact_to_toml(compact_str, source);
}


void VendorChangeManager::load_policies() {
    p_impl->load_policies();
}

}  // namespace libdnf5::base
