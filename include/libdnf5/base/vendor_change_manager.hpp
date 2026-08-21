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

    /// Unload vendor change policies whose source matches the specified pattern.
    /// The pattern may contain glob wildcards.
    /// @param source_pattern A glob pattern to match against policy sources.
    /// @return The number of policies that were unloaded.
    /// @throws VendorChangeManagerError if the pattern is invalid.
    std::size_t unload_policies_matching_source(const std::string & source_pattern);

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
