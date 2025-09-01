// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "libdnf5/base/goal_elements.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-lib.h"

namespace libdnf5 {

class ResolveSpecSettings::Impl {
    friend ResolveSpecSettings;
    bool ignore_case{false};
    bool with_nevra{true};
    bool with_provides{true};
    bool with_filenames{true};
    bool with_binaries{true};
    bool expand_globs{true};
    bool group_with_id{true};
    bool group_with_name{false};
    bool group_search_groups{true};
    bool group_search_environments{true};
    std::vector<libdnf5::rpm::Nevra::Form> nevra_forms{};
};

ResolveSpecSettings::~ResolveSpecSettings() = default;

ResolveSpecSettings::ResolveSpecSettings() : p_impl(std::make_unique<Impl>()) {}
ResolveSpecSettings::ResolveSpecSettings(const ResolveSpecSettings & src) : p_impl(new Impl(*src.p_impl)) {}
ResolveSpecSettings::ResolveSpecSettings(ResolveSpecSettings && src) noexcept = default;

ResolveSpecSettings & ResolveSpecSettings::operator=(const ResolveSpecSettings & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
ResolveSpecSettings & ResolveSpecSettings::operator=(ResolveSpecSettings && src) noexcept = default;

void ResolveSpecSettings::set_ignore_case(bool ignore_case) {
    p_impl->ignore_case = ignore_case;
}
bool ResolveSpecSettings::get_ignore_case() const {
    return p_impl->ignore_case;
}

void ResolveSpecSettings::set_with_nevra(bool with_nevra) {
    p_impl->with_nevra = with_nevra;
}
bool ResolveSpecSettings::get_with_nevra() const {
    return p_impl->with_nevra;
}

void ResolveSpecSettings::set_with_provides(bool with_provides) {
    p_impl->with_provides = with_provides;
}
bool ResolveSpecSettings::get_with_provides() const {
    return p_impl->with_provides;
}

void ResolveSpecSettings::set_with_filenames(bool with_filenames) {
    p_impl->with_filenames = with_filenames;
}
bool ResolveSpecSettings::get_with_filenames() const {
    return p_impl->with_filenames;
}

void ResolveSpecSettings::set_with_binaries(bool with_binaries) {
    p_impl->with_binaries = with_binaries;
}
bool ResolveSpecSettings::get_with_binaries() const {
    return p_impl->with_binaries;
}

void ResolveSpecSettings::set_expand_globs(bool expand_globs) {
    p_impl->expand_globs = expand_globs;
}
bool ResolveSpecSettings::get_expand_globs() const {
    return p_impl->expand_globs;
}

void ResolveSpecSettings::set_nevra_forms(std::vector<libdnf5::rpm::Nevra::Form> nevra_forms) {
    p_impl->nevra_forms = std::move(nevra_forms);
}
std::vector<libdnf5::rpm::Nevra::Form> ResolveSpecSettings::get_nevra_forms() const {
    return p_impl->nevra_forms;
}

void ResolveSpecSettings::set_group_with_id(bool group_with_id) {
    p_impl->group_with_id = group_with_id;
}
bool ResolveSpecSettings::get_group_with_id() const {
    return p_impl->group_with_id;
}

void ResolveSpecSettings::set_group_with_name(bool group_with_name) {
    p_impl->group_with_name = group_with_name;
}
bool ResolveSpecSettings::get_group_with_name() const {
    return p_impl->group_with_name;
}

void ResolveSpecSettings::set_group_search_groups(bool search_groups) {
    p_impl->group_search_groups = search_groups;
}
bool ResolveSpecSettings::get_group_search_groups() const {
    return p_impl->group_search_groups;
}

void ResolveSpecSettings::set_group_search_environments(bool search_environments) {
    p_impl->group_search_environments = search_environments;
}
bool ResolveSpecSettings::get_group_search_environments() const {
    return p_impl->group_search_environments;
}

class GoalJobSettings::Impl {
    friend GoalJobSettings;
    /// If set to true, group operations (install / remove / upgrade) will only work
    /// with the group itself, but will not add to the transaction any packages.
    bool group_no_packages{false};

    /// If set to true, environment operations (install / remove / upgrade) will only work
    /// with the environment itself, but will not add any groups to the transaction.
    bool environment_no_groups{false};

    /// Set whether hints should be reported
    bool report_hint{true};

    /// Set skip_broken, AUTO means that it is handled according to the default behavior
    /// If set resolve depsolve problems by removing packages that are causing them from the transaction.
    GoalSetting skip_broken{GoalSetting::AUTO};

    ///// Set skip_unavailable, AUTO means that it is handled according to the default behavior
    ///// If set and some packages stored in the transaction are not available on the target system,
    ///// skip them instead of erroring out.
    GoalSetting skip_unavailable{GoalSetting::AUTO};

    ///// Set best, AUTO means that it is handled according to the default behavior
    GoalSetting best{GoalSetting::AUTO};

    ///// Set clean_requirements_on_remove, AUTO means that it is handled according to the default behavior
    GoalSetting clean_requirements_on_remove{GoalSetting::AUTO};

    ///// Define which installed packages should be modified according to repoid from which they were installed
    std::vector<std::string> from_repo_ids;
    ///// Reduce candidates for the operation according repository ids
    std::vector<std::string> to_repo_ids;

    /// For replaying transactions don't check for extra packages pulled into the transaction.
    /// Used by history undo, system upgrade, ...
    bool ignore_extras{false};

    /// For replaying transactions don't check for installed packages matching those in transaction.
    /// Used by history undo, system upgrade, ...
    bool ignore_installed{false};

    /// For replaying transactions override reasons for already installed packages.
    /// Used by history redo.
    bool override_reasons{false};

    GoalUsedSetting used_skip_broken{GoalUsedSetting::UNUSED};
    GoalUsedSetting used_skip_unavailable{GoalUsedSetting::UNUSED};
    GoalUsedSetting used_best{GoalUsedSetting::UNUSED};
    GoalUsedSetting used_clean_requirements_on_remove{GoalUsedSetting::UNUSED};
    std::optional<libdnf5::comps::PackageType> used_group_package_types{std::nullopt};
    std::optional<libdnf5::advisory::AdvisoryQuery> advisory_filter{std::nullopt};
    std::optional<libdnf5::comps::PackageType> group_package_types{std::nullopt};
};

GoalJobSettings::~GoalJobSettings() = default;
GoalJobSettings::GoalJobSettings() : p_impl(std::make_unique<Impl>()) {}
GoalJobSettings::GoalJobSettings(const GoalJobSettings & src)
    : ResolveSpecSettings(src),
      p_impl(new Impl(*src.p_impl)) {}
GoalJobSettings::GoalJobSettings(GoalJobSettings && src) noexcept = default;

GoalJobSettings & GoalJobSettings::operator=(const GoalJobSettings & src) {
    ResolveSpecSettings::operator=(src);
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
GoalJobSettings & GoalJobSettings::operator=(GoalJobSettings && src) noexcept = default;


bool GoalJobSettings::resolve_skip_broken(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (p_impl->skip_broken) {
        case GoalSetting::AUTO: {
            bool skip_broken = cfg_main.get_skip_broken_option().get_value();
            resolved = skip_broken ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        p_impl->used_skip_broken == GoalUsedSetting::UNUSED || resolved == p_impl->used_skip_broken,
        "\"skip_broken\" is already set to a different value");

    p_impl->used_skip_broken = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_skip_unavailable(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (p_impl->skip_unavailable) {
        case GoalSetting::AUTO: {
            bool skip_unavailable = cfg_main.get_skip_unavailable_option().get_value();
            resolved = skip_unavailable ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        p_impl->used_skip_unavailable == GoalUsedSetting::UNUSED || resolved == p_impl->used_skip_unavailable,
        "\"skip_unavailable\" is already set to a different value");

    p_impl->used_skip_unavailable = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_skip_broken() {
    bool skip_broken_bool = p_impl->skip_broken == GoalSetting::SET_TRUE;
    auto resolved = skip_broken_bool ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;

    libdnf_assert(
        p_impl->used_skip_broken == GoalUsedSetting::UNUSED || resolved == p_impl->used_skip_broken,
        "Used value for 'used_skip_broken' already set");

    p_impl->used_skip_broken = resolved;

    return skip_broken_bool;
}

bool GoalJobSettings::resolve_best(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (p_impl->best) {
        case GoalSetting::AUTO: {
            bool best = cfg_main.get_best_option().get_value();
            resolved = best ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        p_impl->used_best == GoalUsedSetting::UNUSED || resolved == p_impl->used_best,
        "'best' is already set to a different value");

    p_impl->used_best = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_clean_requirements_on_remove(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = GoalUsedSetting::UNUSED;
    switch (p_impl->clean_requirements_on_remove) {
        case GoalSetting::AUTO: {
            bool clean_requirements_on_remove = cfg_main.get_clean_requirements_on_remove_option().get_value();
            resolved = clean_requirements_on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;
        } break;
        case GoalSetting::SET_TRUE:
            resolved = GoalUsedSetting::USED_TRUE;
            break;
        case GoalSetting::SET_FALSE:
            resolved = GoalUsedSetting::USED_FALSE;
            break;
    }

    libdnf_assert(
        p_impl->used_clean_requirements_on_remove == GoalUsedSetting::UNUSED ||
            resolved == p_impl->used_clean_requirements_on_remove,
        "'clean_requirements_on_remove' is already set to a different value");

    p_impl->used_clean_requirements_on_remove = resolved;
    return resolved == GoalUsedSetting::USED_TRUE;
}

bool GoalJobSettings::resolve_clean_requirements_on_remove() {
    bool on_remove = p_impl->clean_requirements_on_remove == GoalSetting::SET_TRUE;
    auto resolved = on_remove ? GoalUsedSetting::USED_TRUE : GoalUsedSetting::USED_FALSE;

    libdnf_assert(
        p_impl->used_clean_requirements_on_remove == GoalUsedSetting::UNUSED ||
            resolved == p_impl->used_clean_requirements_on_remove,
        "Used value for 'used_clean_requirements_on_remove' already set");

    p_impl->used_clean_requirements_on_remove = resolved;

    return on_remove;
}

libdnf5::comps::PackageType GoalJobSettings::resolve_group_package_types(const libdnf5::ConfigMain & cfg_main) {
    auto resolved = p_impl->group_package_types;
    if (!resolved) {
        resolved = libdnf5::comps::package_type_from_string(cfg_main.get_group_package_types_option().get_value());
    }
    libdnf_assert(
        !p_impl->used_group_package_types || p_impl->used_group_package_types == resolved,
        "Used value for 'used_group_package_types' already set");

    p_impl->used_group_package_types = resolved;

    return *p_impl->used_group_package_types;
}

std::string goal_action_to_string(GoalAction action) {
    switch (action) {
        case GoalAction::INSTALL:
            return _("Install");
        case GoalAction::INSTALL_VIA_PROVIDE:
            return _("Install via provide");
        case GoalAction::INSTALL_BY_COMPS:
            return _("Install by group");
        case GoalAction::UPGRADE:
            return _("Upgrade");
        case GoalAction::UPGRADE_ALL:
            return _("Upgrade all");
        case GoalAction::UPGRADE_MINIMAL:
            return _("Upgrade minimal");
        case GoalAction::UPGRADE_ALL_MINIMAL:
            return _("Upgrade all minimal");
        case GoalAction::DOWNGRADE:
            return _("Downgrade");
        case GoalAction::REINSTALL:
            return _("Reinstall");
        case GoalAction::INSTALL_OR_REINSTALL:
            return _("Install or reinstall");
        case GoalAction::REMOVE:
            return _("Remove");
        case GoalAction::DISTRO_SYNC:
            return _("Distrosync");
        case GoalAction::DISTRO_SYNC_ALL:
            return _("Distrosync all");
        case GoalAction::REASON_CHANGE:
            return _("Reason Change");
        case GoalAction::RESOLVE:
            return _("Resolve");
        case GoalAction::ENABLE:
            return _("Enable");
        case GoalAction::DISABLE:
            return _("Disable");
        case GoalAction::RESET:
            return _("Reset");
        case GoalAction::REPLAY_PARSE:
            return _("Parse serialized transaction");
        case GoalAction::REPLAY_INSTALL:
            return _("Install action");
        case GoalAction::REPLAY_REMOVE:
            return _("Remove action");
        case GoalAction::REPLAY_UPGRADE:
            return _("Upgrade action");
        case GoalAction::REPLAY_REINSTALL:
            return _("Reinstall action");
        case GoalAction::REPLAY_REASON_CHANGE:
            return _("Reason change action");
        case GoalAction::REPLAY_REASON_OVERRIDE:
            return _("Reason override");
        case GoalAction::REVERT_COMPS_UPGRADE:
            return _("Revert comps upgrade");
        case GoalAction::INSTALL_DEBUG:
            return _("Install debug RPMs");
        case GoalAction::MERGE:
            return _("Transaction merge");
    }
    return "";
}

bool goal_action_is_replay(GoalAction action) {
    if (action == GoalAction::REPLAY_INSTALL || action == GoalAction::REPLAY_REMOVE ||
        action == GoalAction::REPLAY_UPGRADE || action == GoalAction::REPLAY_REINSTALL ||
        action == GoalAction::REPLAY_REASON_CHANGE || action == GoalAction::REPLAY_REASON_OVERRIDE) {
        return true;
    } else {
        return false;
    }
}

void GoalJobSettings::set_report_hint(bool report_hint) {
    p_impl->report_hint = report_hint;
}
bool GoalJobSettings::get_report_hint() const {
    return p_impl->report_hint;
}

void GoalJobSettings::set_skip_broken(GoalSetting skip_broken) {
    p_impl->skip_broken = skip_broken;
}
GoalSetting GoalJobSettings::get_skip_broken() const {
    return p_impl->skip_broken;
}

void GoalJobSettings::set_skip_unavailable(GoalSetting skip_unavailable) {
    p_impl->skip_unavailable = skip_unavailable;
}
GoalSetting GoalJobSettings::get_skip_unavailable() const {
    return p_impl->skip_unavailable;
}

void GoalJobSettings::set_best(GoalSetting best) {
    p_impl->best = best;
}
GoalSetting GoalJobSettings::get_best() const {
    return p_impl->best;
}

void GoalJobSettings::set_clean_requirements_on_remove(GoalSetting clean_requirements_on_remove) {
    p_impl->clean_requirements_on_remove = clean_requirements_on_remove;
}
GoalSetting GoalJobSettings::get_clean_requirements_on_remove() const {
    return p_impl->clean_requirements_on_remove;
}

void GoalJobSettings::set_from_repo_ids(std::vector<std::string> from_repo_ids) {
    p_impl->from_repo_ids = std::move(from_repo_ids);
}
std::vector<std::string> GoalJobSettings::get_from_repo_ids() const {
    return p_impl->from_repo_ids;
}

void GoalJobSettings::set_to_repo_ids(std::vector<std::string> to_repo_ids) {
    p_impl->to_repo_ids = std::move(to_repo_ids);
}
std::vector<std::string> GoalJobSettings::get_to_repo_ids() const {
    return p_impl->to_repo_ids;
}

void GoalJobSettings::set_group_no_packages(bool group_no_packages) {
    p_impl->group_no_packages = group_no_packages;
}
bool GoalJobSettings::get_group_no_packages() const {
    return p_impl->group_no_packages;
}

void GoalJobSettings::set_environment_no_groups(bool environment_no_groups) {
    p_impl->environment_no_groups = environment_no_groups;
}
bool GoalJobSettings::get_environment_no_groups() const {
    return p_impl->environment_no_groups;
}

void GoalJobSettings::set_advisory_filter(const libdnf5::advisory::AdvisoryQuery & filter) {
    p_impl->advisory_filter = filter;
};
const libdnf5::advisory::AdvisoryQuery * GoalJobSettings::get_advisory_filter() const {
    return p_impl->advisory_filter ? &p_impl->advisory_filter.value() : nullptr;
}

void GoalJobSettings::set_group_package_types(const libdnf5::comps::PackageType type) {
    p_impl->group_package_types = type;
}
const libdnf5::comps::PackageType * GoalJobSettings::get_group_package_types() const {
    return p_impl->group_package_types ? &p_impl->group_package_types.value() : nullptr;
}

GoalUsedSetting GoalJobSettings::get_used_skip_broken() const {
    return p_impl->used_skip_broken;
};
GoalUsedSetting GoalJobSettings::get_used_skip_unavailable() const {
    return p_impl->used_skip_unavailable;
};
GoalUsedSetting GoalJobSettings::get_used_best() const {
    return p_impl->used_best;
};
GoalUsedSetting GoalJobSettings::get_used_clean_requirements_on_remove() const {
    return p_impl->used_clean_requirements_on_remove;
};

void GoalJobSettings::set_ignore_extras(bool ignore_extras) {
    p_impl->ignore_extras = ignore_extras;
}
bool GoalJobSettings::get_ignore_extras() const {
    return p_impl->ignore_extras;
}

void GoalJobSettings::set_ignore_installed(bool ignore_installed) {
    p_impl->ignore_installed = ignore_installed;
}
bool GoalJobSettings::get_ignore_installed() const {
    return p_impl->ignore_installed;
}

void GoalJobSettings::set_override_reasons(bool override_reasons) {
    p_impl->override_reasons = override_reasons;
}
bool GoalJobSettings::get_override_reasons() const {
    return p_impl->override_reasons;
}

}  // namespace libdnf5
