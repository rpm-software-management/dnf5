// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/vendor_change_manager.hpp"

#include "base_impl.hpp"
#include "conf/config.h"
#include "solv/pool.hpp"
#include "solv/vendor_change_manager.hpp"
#include "utils/fs/utils.hpp"

#include "libdnf5/base/vendor_change_manager_errors.hpp"
#include "libdnf5/conf/const.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"

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

    void save_policy_from_toml(
        std::string_view toml_content, std::string_view source, const std::filesystem::path & base_filename);

    void save_policy_from_toml(const std::filesystem::path & path, const std::filesystem::path & base_filename);

    void save_policy_from_compact(
        std::string_view policy_str, std::string_view source, const std::filesystem::path & base_filename);

    void remove_policy_file(const std::filesystem::path & base_filename);

    std::vector<std::filesystem::path> get_policy_files() const;

    bool is_policy_file_manageable(const std::filesystem::path & path) const;

    std::filesystem::path get_masked_policy_file(const std::filesystem::path & path) const;

    std::filesystem::path find_policy_file(const std::filesystem::path & base_filename) const;

    WeakPtrGuard<VendorChangeManager, false> guard;

private:
    void save_policy_file(
        std::string_view toml_content, const std::filesystem::path & base_filename, const char * caller_name);

    std::filesystem::path get_vendor_conf_dir_path() const;
    std::filesystem::path get_distribution_vendor_conf_dir_path() const;
    static std::filesystem::path make_policy_filename(const std::filesystem::path & base_filename);

    Base & base;
    solv::VendorChangeManager & vcm;
};


void VendorChangeManager::Impl::load_policies() {
    for (const auto & path : get_policy_files()) {
        vcm.add_policy_from_toml(path);
    }
}


void VendorChangeManager::Impl::save_policy_from_toml(
    std::string_view toml_content, std::string_view source, const std::filesystem::path & base_filename) {
    // Validate TOML by parsing it
    solv::VendorChangeManager::convert_policy_toml_to_compact(toml_content, source);
    save_policy_file(toml_content, base_filename, __func__);
}


void VendorChangeManager::Impl::save_policy_from_toml(
    const std::filesystem::path & path, const std::filesystem::path & base_filename) {
    // Read TOML content from file
    const std::string toml_content = utils::fs::File(path, "rb").read();
    // Validate TOML by parsing it
    solv::VendorChangeManager::convert_policy_toml_to_compact(toml_content, "file://" + path.string());
    save_policy_file(toml_content, base_filename, __func__);
}


void VendorChangeManager::Impl::save_policy_from_compact(
    std::string_view policy_str, std::string_view source, const std::filesystem::path & base_filename) {
    const auto toml_content = solv::VendorChangeManager::convert_policy_compact_to_toml(policy_str, source);
    save_policy_file(toml_content, base_filename, __func__);
}


void VendorChangeManager::Impl::remove_policy_file(const std::filesystem::path & base_filename) {
    namespace fs = std::filesystem;

    fs::path file_path = get_vendor_conf_dir_path() / make_policy_filename(base_filename);

    std::error_code ec;
    if (!fs::remove(file_path, ec)) {
        if (ec) {
            throw VendorChangeManagerError(
                M_("{func}(): Cannot remove file: {path}: {msg}"),
                NamedErrorArg("func", __func__),
                NamedErrorArg("path", file_path.string()),
                NamedErrorArg("msg", ec.message()));
        } else {
            throw VendorChangeManagerError(
                M_("{func}(): File does not exist: {path}"),
                NamedErrorArg("func", __func__),
                NamedErrorArg("path", file_path.string()));
        }
    }
}


std::vector<std::filesystem::path> VendorChangeManager::Impl::get_policy_files() const {
    return utils::fs::create_sorted_file_list(
        {get_vendor_conf_dir_path(), get_distribution_vendor_conf_dir_path()}, ".conf");
}


bool VendorChangeManager::Impl::is_policy_file_manageable(const std::filesystem::path & path) const {
    namespace fs = std::filesystem;

    // Must have .conf extension to be manageable
    if (path.extension() != ".conf") {
        return false;
    }

    // Check if the file's parent directory is equivalent to the vendor config directory
    std::error_code ec;
    return fs::equivalent(path.parent_path(), get_vendor_conf_dir_path(), ec);
}


std::filesystem::path VendorChangeManager::Impl::get_masked_policy_file(const std::filesystem::path & path) const {
    namespace fs = std::filesystem;

    // Only manageable files can mask other files
    if (!is_policy_file_manageable(path)) {
        return {};
    }

    // Build path to potentially masked file in distribution directory
    fs::path masked_file_path = get_distribution_vendor_conf_dir_path() / path.filename();

    // Check if the masked file exists
    std::error_code ec;
    if (fs::exists(masked_file_path, ec)) {
        return masked_file_path;
    }

    return {};
}


std::filesystem::path VendorChangeManager::Impl::find_policy_file(const std::filesystem::path & base_filename) const {
    namespace fs = std::filesystem;

    const auto filename = make_policy_filename(base_filename);

    // Try vendor config directory first
    fs::path vendor_file = get_vendor_conf_dir_path() / filename;
    std::error_code ec;
    if (fs::exists(vendor_file, ec)) {
        return vendor_file;
    }

    // Try distribution config directory
    fs::path dist_file = get_distribution_vendor_conf_dir_path() / filename;
    if (fs::exists(dist_file, ec)) {
        return dist_file;
    }

    return {};
}


void VendorChangeManager::Impl::save_policy_file(
    std::string_view toml_content, const std::filesystem::path & base_filename, const char * caller_name) {
    namespace fs = std::filesystem;

    fs::path file_path = get_vendor_conf_dir_path() / make_policy_filename(base_filename);

    // Use "wx" mode to fail if file already exists
    utils::fs::File file;
    try {
        file.open(file_path, "wxb");
    } catch (...) {
        libdnf5::throw_with_nested(VendorChangeManagerError(
            M_("{func}(): Cannot create file: {path}"),
            NamedErrorArg("func", caller_name),
            NamedErrorArg("path", file_path.string())));
    }

    try {
        file.write(toml_content);
    } catch (...) {
        // Write failed - close and remove the partially written file
        try {
            file.close();
        } catch (...) {
        }
        std::error_code ec;
        fs::remove(file_path, ec);

        libdnf5::throw_with_nested(VendorChangeManagerError(
            M_("{func}(): Cannot write to file: {path}"),
            NamedErrorArg("func", caller_name),
            NamedErrorArg("path", file_path.string())));
    }
}


std::filesystem::path VendorChangeManager::Impl::get_vendor_conf_dir_path() const {
    namespace fs = std::filesystem;

    fs::path vendor_conf_dir_path{VENDOR_CONF_DIR};
    const bool use_installroot_config{!base.get_config().get_use_host_config_option().get_value()};
    if (use_installroot_config) {
        fs::path installroot_path{base.get_config().get_installroot_option().get_value()};
        vendor_conf_dir_path = installroot_path / vendor_conf_dir_path.relative_path();
    }
    return vendor_conf_dir_path;
}


std::filesystem::path VendorChangeManager::Impl::get_distribution_vendor_conf_dir_path() const {
    namespace fs = std::filesystem;

    fs::path distribution_vendor_conf_dir_path{LIBDNF5_DISTRIBUTION_VENDOR_CONF_DIR};
    const bool use_installroot_config{!base.get_config().get_use_host_config_option().get_value()};
    if (use_installroot_config) {
        fs::path installroot_path{base.get_config().get_installroot_option().get_value()};
        distribution_vendor_conf_dir_path = installroot_path / distribution_vendor_conf_dir_path.relative_path();
    }
    return distribution_vendor_conf_dir_path;
}


std::filesystem::path VendorChangeManager::Impl::make_policy_filename(const std::filesystem::path & base_filename) {
    if (base_filename.empty()) {
        throw VendorChangeManagerError(M_("{func}(): Base filename cannot be empty"), NamedErrorArg("func", __func__));
    }

    // Security check: prevent path traversal attacks
    if (base_filename.has_parent_path()) {
        throw VendorChangeManagerError(
            M_("{func}(): Base filename must not contain path components: {base_filename}"),
            NamedErrorArg("func", __func__),
            NamedErrorArg("base_filename", base_filename.string()));
    }

    std::filesystem::path filename = base_filename;
    filename += ".conf";
    return filename;
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


void VendorChangeManager::save_policy_from_toml(
    std::string_view toml_content, std::string_view source, const std::filesystem::path & base_filename) {
    p_impl->save_policy_from_toml(toml_content, source, base_filename);
}


void VendorChangeManager::save_policy_from_toml(
    const std::filesystem::path & path, const std::filesystem::path & base_filename) {
    p_impl->save_policy_from_toml(path, base_filename);
}


void VendorChangeManager::save_policy_from_compact(
    std::string_view policy_str, std::string_view source, const std::filesystem::path & base_filename) {
    p_impl->save_policy_from_compact(policy_str, source, base_filename);
}


void VendorChangeManager::remove_policy_file(const std::filesystem::path & base_filename) {
    p_impl->remove_policy_file(base_filename);
}


std::vector<std::filesystem::path> VendorChangeManager::get_policy_files() const {
    return p_impl->get_policy_files();
}


std::filesystem::path VendorChangeManager::extract_policy_base_filename(const std::filesystem::path & path) {
    if (path.extension() != ".conf") {
        throw VendorChangeManagerError(
            M_("{func}(): Path must have .conf extension: {path}"),
            NamedErrorArg("func", __func__),
            NamedErrorArg("path", path.string()));
    }
    return path.stem();
}


bool VendorChangeManager::is_policy_file_manageable(const std::filesystem::path & path) const {
    return p_impl->is_policy_file_manageable(path);
}


std::filesystem::path VendorChangeManager::get_masked_policy_file(const std::filesystem::path & path) const {
    return p_impl->get_masked_policy_file(path);
}


std::filesystem::path VendorChangeManager::find_policy_file(const std::filesystem::path & base_filename) const {
    return p_impl->find_policy_file(base_filename);
}


void VendorChangeManager::load_policies() {
    p_impl->load_policies();
}

}  // namespace libdnf5::base
