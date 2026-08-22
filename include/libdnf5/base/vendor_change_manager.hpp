// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_HPP
#define LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_HPP

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/defs.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace libdnf5 {

class Base;

namespace base {

class VendorChangeManager;
using VendorChangeManagerWeakPtr = WeakPtr<VendorChangeManager, false>;

/// Manages vendor change policies that control which package vendor transitions are allowed.
///
/// Policies can be loaded from TOML configuration files (typically found in
/// ``/etc/dnf/vendors.d/`` and ``/usr/share/dnf5/vendors.d/``) or from a compact
/// string representation suitable for command-line use.
///
/// Policies loaded from configuration files during :func:`Base::setup()` are applied
/// automatically. Additional policies can be added at runtime via this class.
///
/// An instance is obtained from :func:`Base::get_vendor_change_manager()` after
/// :func:`Base::setup()` has been called.
class LIBDNF_API VendorChangeManager {
public:
    ~VendorChangeManager();

    VendorChangeManager(const VendorChangeManager &) = delete;
    VendorChangeManager & operator=(const VendorChangeManager &) = delete;

    /// @return A weak pointer to this VendorChangeManager instance.
    VendorChangeManagerWeakPtr get_weak_ptr();

    /// Load a vendor change policy from TOML content.
    /// The content must conform to the vendor change policy format.
    /// @param toml_content The TOML content to parse.
    /// @param source Origin of the policy (file URI or custom label with ``text:`` prefix,
    ///               e.g., ``"text:COMMAND LINE"``). Stored for diagnostics and logging.
    /// @throws VendorChangeManagerError if the content cannot be parsed or contains invalid values.
    void load_policy_from_toml(std::string_view toml_content, std::string_view source);

    /// Load one vendor change policy from a TOML configuration file.
    /// The file must conform to the vendor change policy format.
    /// @param path Path to the TOML configuration file.
    /// @throws VendorChangeManagerError if the file cannot be parsed or contains invalid values.
    void load_policy_from_toml(const std::filesystem::path & path);

    /// Load a vendor change policy from a compact string representation.
    ///
    /// The compact format is: ``direction:eop"value",...@direction:e[filters],...``
    ///
    /// Both sections (before and after ``@``) are optional, but at least one must be present.
    /// Vendor entries use direction prefixes ``in:``, ``out:``, or ``eq:`` followed by
    /// an optional ``e`` (exclude) flag and an operator with a quoted value.
    /// Package filter entries are enclosed in literal ``[...]`` after the ``@`` separator.
    ///
    /// Example: ``eq:i^"Fedora",in:"Third Vendor"@in:[version>="2.0"]``
    ///
    /// @param policy_str The compact policy string to parse.
    /// @param source Origin of the policy (file URI or custom label with ``text:`` prefix,
    ///               e.g., ``"text:COMMAND LINE"``). Stored for diagnostics and logging.
    /// @throws VendorChangeManagerError if the string cannot be parsed.
    void load_policy_from_compact(std::string_view policy_str, std::string_view source);

    /// Unload all vendor change policies from memory.
    /// @return The number of policies that were unloaded.
    std::size_t unload_policies();

    /// Unload a vendor change policy at the specified index.
    /// @param index The index of the policy to unload (0-based).
    /// @throws VendorChangeManagerError if index is out of bounds.
    void unload_policy(std::size_t index);

    /// Unload vendor change policies whose source matches the specified pattern.
    /// The pattern may contain glob wildcards.
    /// @param source_pattern A glob pattern to match against policy sources.
    /// @return The number of policies that were unloaded.
    /// @throws VendorChangeManagerError if the pattern is invalid.
    std::size_t unload_policies_matching_source(const std::string & source_pattern);

    /// Get the number of loaded vendor change policies.
    /// @return The number of currently loaded policies.
    [[nodiscard]] std::size_t get_loaded_policies_count() const noexcept;

    /// Get the source (origin) of a loaded vendor change policy.
    /// @param index The index of the loaded policy (0-based).
    /// @return The source string (file URI or custom label with ``text:`` prefix).
    /// @throws VendorChangeManagerError if index is out of bounds.
    [[nodiscard]] const std::string & get_loaded_policy_source(std::size_t index) const;

    /// Get a loaded vendor change policy as a TOML string.
    /// @param index The index of the loaded policy (0-based).
    /// @return The policy formatted as a TOML string.
    /// @throws VendorChangeManagerError if index is out of bounds.
    [[nodiscard]] std::string get_loaded_policy_as_toml(std::size_t index) const;

    /// Get a loaded vendor change policy as a compact format string.
    /// @param index The index of the loaded policy (0-based).
    /// @return The policy formatted as a compact string.
    /// @throws VendorChangeManagerError if index is out of bounds.
    [[nodiscard]] std::string get_loaded_policy_as_compact(std::size_t index) const;

    /// Convert a vendor change policy from TOML content to compact format.
    /// @param toml_content The TOML content to parse.
    /// @param source Origin of the policy (for error messages).
    /// @return The policy formatted as a compact string.
    /// @throws VendorChangeManagerError if parsing fails.
    static std::string convert_policy_toml_to_compact(std::string_view toml_content, std::string_view source);

    /// Convert a vendor change policy from TOML file to compact format.
    /// @param path Path to the TOML configuration file.
    /// @return The policy formatted as a compact string.
    /// @throws VendorChangeManagerError if parsing fails.
    static std::string convert_policy_toml_to_compact(const std::filesystem::path & path);

    /// Convert a vendor change policy from compact format to TOML format.
    /// @param compact_str The policy in compact format.
    /// @param source Origin of the policy (for error messages).
    /// @return The policy formatted as a TOML string.
    /// @throws VendorChangeManagerError if parsing fails.
    static std::string convert_policy_compact_to_toml(std::string_view compact_str, std::string_view source);

    /// Save a vendor change policy from TOML format to a configuration file
    /// in the vendor directory (``/etc/dnf/vendors.d/`` or installroot equivalent).
    /// The TOML content is validated by parsing before saving.
    /// @param toml_content The policy in TOML format.
    /// @param source Origin of the policy (for error messages and validation).
    /// @param base_filename Base filename without extension or path (e.g., ``"my-policy"``).
    ///                      Must not contain path components. The ``.conf`` extension is added automatically.
    /// @throws VendorChangeManagerError if validation or file write fails,
    ///                                  or if base_filename contains path components.
    void save_policy_from_toml(
        std::string_view toml_content, std::string_view source, const std::filesystem::path & base_filename);

    /// Save a vendor change policy from a TOML file to a configuration file
    /// in the vendor directory (``/etc/dnf/vendors.d/`` or installroot equivalent).
    /// The TOML file content is validated by parsing before saving.
    /// @param path Path to the TOML file containing the policy.
    /// @param base_filename Base filename without extension or path (e.g., ``"my-policy"``).
    ///                      Must not contain path components. The ``.conf`` extension is added automatically.
    /// @throws VendorChangeManagerError if file read, validation, or write fails,
    ///                                  or if base_filename contains path components.
    void save_policy_from_toml(const std::filesystem::path & path, const std::filesystem::path & base_filename);

    /// Save a vendor change policy from compact format to a configuration file
    /// in the vendor directory (``/etc/dnf/vendors.d/`` or installroot equivalent).
    /// The policy is converted from compact format to TOML before saving.
    /// @param policy_str The policy in compact format.
    /// @param source Origin of the policy (for error messages).
    /// @param base_filename Base filename without extension or path (e.g., ``"my-policy"``).
    ///                      Must not contain path components. The ``.conf`` extension is added automatically.
    /// @throws VendorChangeManagerError if conversion or file write fails,
    ///                                  or if base_filename contains path components.
    void save_policy_from_compact(
        std::string_view policy_str, std::string_view source, const std::filesystem::path & base_filename);

    /// Remove a vendor change policy configuration file.
    /// Removes the file from the vendor configuration directory (``/etc/dnf/vendors.d/`` or installroot equivalent).
    /// @param base_filename Base filename without extension or path (e.g., ``"my-policy"``).
    ///                      Must not contain path components. The ``.conf`` extension is added automatically.
    /// @throws VendorChangeManagerError if base_filename contains path components or file removal fails.
    void remove_policy_file(const std::filesystem::path & base_filename);

    /// Get a list of all vendor change policy configuration files.
    /// Returns paths to all ``.conf`` files from both vendor configuration directories
    /// (``/etc/dnf/vendors.d/`` and ``/usr/share/dnf5/vendors.d/`` or their installroot equivalents).
    /// The list is sorted in the same order as policies are loaded.
    /// @return Vector of paths to policy configuration files.
    [[nodiscard]] std::vector<std::filesystem::path> get_policy_files() const;

    /// Extract base filename from a policy file path.
    /// Returns the filename without directory path and without the ``.conf`` extension.
    /// This is useful for converting paths returned by :func:`get_policy_files()` to the format
    /// expected by :func:`remove_policy_file()` and :func:`save_policy_from_compact()`.
    /// @param path Path to a policy file (e.g., ``"/etc/dnf/vendors.d/my-policy.conf"``).
    ///             Must have ``.conf`` extension.
    /// @return Base filename without extension (e.g., ``"my-policy"``).
    /// @throws VendorChangeManagerError if path does not have ``.conf`` extension.
    [[nodiscard]] static std::filesystem::path extract_policy_base_filename(const std::filesystem::path & path);

    /// Check if a policy file is manageable (can be removed or modified).
    /// Returns ``true`` if the file is in the writable vendor configuration directory
    /// (``/etc/dnf/vendors.d/`` or installroot equivalent). Files in the distribution
    /// directory (``/usr/share/dnf5/vendors.d/``) are system-provided and not manageable.
    /// @param path Path to a policy file.
    /// @return ``true`` if the file can be managed (removed/modified), ``false`` otherwise.
    [[nodiscard]] bool is_policy_file_manageable(const std::filesystem::path & path) const;

    /// Get the path to the policy file that is masked by the given file.
    /// A file in the vendor configuration directory (``/etc/dnf/vendors.d/``) masks
    /// a file with the same name in the distribution directory (``/usr/share/dnf5/vendors.d/``).
    /// @param path Path to a policy file in the vendor configuration directory.
    /// @return Path to the masked file if the masked file exists,
    ///         empty path otherwise (if the masked file does not exist or path is invalid).
    [[nodiscard]] std::filesystem::path get_masked_policy_file(const std::filesystem::path & path) const;

    /// Find an existing policy file by base filename.
    /// Searches for a policy file with the given base filename, preferring the vendor
    /// configuration directory (``/etc/dnf/vendors.d/``) over the distribution directory
    /// (``/usr/share/dnf5/vendors.d/``).
    /// @param base_filename Base filename without extension or path (e.g., ``"my-policy"``).
    /// @return Path to the existing policy file (vendor or distribution),
    ///         empty path if no such file exists.
    [[nodiscard]] std::filesystem::path find_policy_file(const std::filesystem::path & base_filename) const;

private:
    friend class libdnf5::Base;
    explicit VendorChangeManager(Base & base);

    /// Load vendor change policies from configuration directories.
    /// Typically called by :func:`Base::setup()` to load policies from
    /// ``/etc/dnf/vendors.d/`` and ``/usr/share/dnf5/vendors.d/``.
    /// The directories are determined from Base configuration.
    LIBDNF_LOCAL void load_policies();

    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace base
}  // namespace libdnf5

#endif  // LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_HPP
