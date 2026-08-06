// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_REPO_CONFIG_OVERRIDE_HPP
#define LIBDNF5_REPO_REPO_CONFIG_OVERRIDE_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <filesystem>
#include <map>
#include <set>
#include <string>


namespace libdnf5::repo {

/// Manages persistent repository configuration overrides in the standard
/// override file (/etc/dnf/repos.override.d/99-config_manager.repo).
class LIBDNF_API RepoConfigOverride {
public:
    explicit RepoConfigOverride(libdnf5::Base & base);
    ~RepoConfigOverride();

    RepoConfigOverride(const RepoConfigOverride & src);
    RepoConfigOverride & operator=(const RepoConfigOverride & src);

    RepoConfigOverride(RepoConfigOverride && src) noexcept;
    RepoConfigOverride & operator=(RepoConfigOverride && src) noexcept;

    /// Returns the path to the repository configuration override file,
    /// taking installroot into account.
    std::filesystem::path get_override_file_path() const;

    /// Persistently modify repository configuration overrides in the standard override file.
    /// Sets and/or removes option overrides for the given repositories.
    /// Creates the override directory and file if they don't exist.
    /// Removes sections that become empty after key removal.
    /// Overrides are applied before removals.
    /// Options in `overrides` are validated against the known repository
    /// options; `removals` are not, so overrides for options that are no
    /// longer known (e.g. after a version change) can still be removed.
    /// @param overrides Map of repo_id -> (option_name -> option_value) to set
    /// @param removals Map of repo_id -> set of option names to remove
    void save(
        const std::map<std::string, std::map<std::string, std::string>> & overrides,
        const std::map<std::string, std::set<std::string>> & removals = {});

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5::repo

#endif
