// Copyright (C) 2021 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "system/state.hpp"

#include "utils/fs/utils.hpp"
#include "utils/string.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"
#include "libdnf5/utils/fs/temp.hpp"

#include <libdnf5/comps/group/package.hpp>
#include <toml.hpp>


namespace toml {

template <>
struct from<libdnf5::system::PackageState> {
    static libdnf5::system::PackageState from_toml(const value & v) {
        libdnf5::system::PackageState pkg_state;

        pkg_state.reason = toml::find<std::string>(v, "reason");

        return pkg_state;
    }
};


template <>
struct into<libdnf5::system::PackageState> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::system::PackageState & pkg_state) {
        toml::value res;
        res["reason"] = pkg_state.reason;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::system::PackageState & pkg_state) {
        toml::basic_value<TC> res;
        res["reason"] = pkg_state.reason;
        res.as_table_fmt().fmt = toml::table_format::oneline;
#endif  // #ifdef TOML11_COMPAT

        return res;
    }
};


template <>
struct from<libdnf5::system::NevraState> {
    static libdnf5::system::NevraState from_toml(const value & v) {
        libdnf5::system::NevraState nevra_state;

        if (v.contains("from_repo")) {
            nevra_state.from_repo = toml::find<std::string>(v, "from_repo");
        }

        return nevra_state;
    }
};


template <>
struct into<libdnf5::system::NevraState> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::system::NevraState & nevra_state) {
        toml::value res;
        res["from_repo"] = nevra_state.from_repo;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::system::NevraState & nevra_state) {
        toml::basic_value<TC> res;
        res["from_repo"] = nevra_state.from_repo;
        res.as_table_fmt().fmt = toml::table_format::oneline;
#endif  // #ifdef TOML11_COMPAT

        return res;
    }
};


template <>
struct from<libdnf5::comps::PackageType> {
    static libdnf5::comps::PackageType from_toml(const value & v) {
        return libdnf5::comps::package_type_from_string(toml::get<std::vector<std::string>>(v));
    }
};


template <>
struct into<libdnf5::comps::PackageType> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::comps::PackageType & package_types) {
        return libdnf5::comps::package_types_to_strings(package_types);
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::comps::PackageType & package_types) {
        toml::basic_value<TC> res = libdnf5::comps::package_types_to_strings(package_types);
        res.as_array_fmt().fmt = toml::array_format::oneline;
        return res;
#endif  // #ifdef TOML11_COMPAT
    }
};

template <>
struct from<libdnf5::system::GroupState> {
    static libdnf5::system::GroupState from_toml(const value & v) {
        libdnf5::system::GroupState group_state;

        group_state.userinstalled = toml::find<bool>(v, "userinstalled");
        if (v.contains("package_types")) {
            group_state.package_types = toml::find<libdnf5::comps::PackageType>(v, "package_types");
        }
        if (v.contains("packages")) {
            group_state.packages = toml::find<std::vector<std::string>>(v, "packages");
        }

        return group_state;
    }
};


template <>
struct into<libdnf5::system::GroupState> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::system::GroupState & group_state) {
        toml::value res;
        res["package_types"] = group_state.package_types;
        res["packages"] = group_state.packages;
        res["userinstalled"] = group_state.userinstalled;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::system::GroupState & group_state) {
        toml::basic_value<TC> res;
        res["package_types"] = group_state.package_types;
        res["packages"] = group_state.packages;
        res["userinstalled"] = group_state.userinstalled;
#endif  // #ifdef TOML11_COMPAT

        return res;
    }
};


template <>
struct from<libdnf5::system::EnvironmentState> {
    static libdnf5::system::EnvironmentState from_toml(const value & v) {
        libdnf5::system::EnvironmentState environment_state;

        if (v.contains("groups")) {
            environment_state.groups = toml::find<std::vector<std::string>>(v, "groups");
        }

        return environment_state;
    }
};


template <>
struct into<libdnf5::system::EnvironmentState> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::system::EnvironmentState & environment_state) {
        toml::value res;
        res["groups"] = environment_state.groups;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::system::EnvironmentState & environment_state) {
        toml::basic_value<TC> res;
        res["groups"] = environment_state.groups;
        res.as_table_fmt().fmt = toml::table_format::oneline;
#endif  // #ifdef TOML11_COMPAT

        return res;
    }
};


#ifdef WITH_MODULEMD
template <>
struct from<libdnf5::system::ModuleState> {
    static libdnf5::system::ModuleState from_toml(const value & v) {
        libdnf5::system::ModuleState module_state;

        module_state.enabled_stream = toml::find<std::string>(v, "enabled_stream");
        module_state.status = libdnf5::module::module_status_from_string(toml::find<std::string>(v, "state"));
        module_state.installed_profiles = toml::find<std::vector<std::string>>(v, "installed_profiles");

        return module_state;
    }
};


template <>
struct into<libdnf5::system::ModuleState> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::system::ModuleState & module_state) {
        toml::value res;
        res["enabled_stream"] = module_state.enabled_stream;
        res["state"] = libdnf5::module::module_status_to_string(module_state.status);
        res["installed_profiles"] = module_state.installed_profiles;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::system::ModuleState & module_state) {
        toml::basic_value<TC> res;
        res["enabled_stream"] = module_state.enabled_stream;
        res["installed_profiles"] = module_state.installed_profiles;
        res["state"] = libdnf5::module::module_status_to_string(module_state.status);
#endif  // #ifdef TOML11_COMPAT

        return res;
    }
};
#endif


template <>
struct from<libdnf5::system::SystemState> {
    static libdnf5::system::SystemState from_toml(const value & v) {
        libdnf5::system::SystemState system_state;

        system_state.rpmdb_cookie = toml::find<std::string>(v, "rpmdb_cookie");

        return system_state;
    }
};


template <>
struct into<libdnf5::system::SystemState> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::system::SystemState & system_state) {
        toml::value res;
        res["rpmdb_cookie"] = system_state.rpmdb_cookie;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::system::SystemState & system_state) {
        toml::basic_value<TC> res;
        res["rpmdb_cookie"] = system_state.rpmdb_cookie;
        res.as_table_fmt().fmt = toml::table_format::oneline;
#endif  // #ifdef TOML11_COMPAT

        return res;
    }
};

}  // namespace toml


namespace libdnf5::system {

const constexpr uint8_t version_major{1};
const constexpr uint8_t version_minor{0};


static std::string make_version() {
    return fmt::format("{}.{}", version_major, version_minor);
}


static std::pair<uint64_t, uint64_t> parse_version(const std::string & version) {
    std::vector<std::string> split_version = libdnf5::utils::string::split(version, ".");
    if (split_version.size() != 2) {
        throw InvalidVersionError(M_("Invalid TOML version \"{}\", \"MAJOR.MINOR\" expected"), version);
    }

    uint64_t major;
    uint64_t minor;
    try {
        major = std::stoul(split_version[0]);
        minor = std::stoul(split_version[1]);
    } catch (const std::exception & e) {
        throw InvalidVersionError(M_("Invalid TOML version \"{}\", \"MAJOR.MINOR\" expected"), version);
    }

    return {major, minor};
}


StateNotFoundError::StateNotFoundError(const std::string & type, const std::string & key)
    : libdnf5::Error(M_("{} state for \"{}\" not found."), type, key) {}


StateLoadError::StateLoadError(const std::string & path, const std::string & error)
    : libdnf5::Error(M_("Loading system state TOML file {} failed (see dnf5-system-state(7)): {}"), path, error) {}


State::State(const libdnf5::BaseWeakPtr & base, const std::filesystem::path & path) : path(path), base(base) {
    load();
}


bool State::packages_import_required() {
    // TODO(mblaha) - detect by absence of toml file instead of empty nevra_states?
    // Because even empty nevra_states is a valid state.
    return nevra_states.empty();
}


transaction::TransactionItemReason State::get_package_reason(const std::string & na) {
    transaction::TransactionItemReason packages_reason = transaction::TransactionItemReason::NONE;
    auto it = package_states.find(na);
    if (it != package_states.end()) {
        // TODO(lukash) this allows more reasons than valid here, assert?
        packages_reason = transaction::transaction_item_reason_from_string(it->second.reason);
    }

    if (packages_reason < transaction::TransactionItemReason::GROUP) {
        // group packages are not stored in packages.toml but in groups.toml
        // using its name
        std::string name = na;
        auto dot_pos = name.find('.');
        if (dot_pos != name.npos) {
            name = name.substr(0, dot_pos);
        }
        if (get_package_groups_cache().contains(name)) {
            return transaction::TransactionItemReason::GROUP;
        }
    }
    return packages_reason;
}

transaction::TransactionItemReason State::get_group_reason(const std::string & id) {
    transaction::TransactionItemReason group_reason = transaction::TransactionItemReason::NONE;
    auto it = group_states.find(id);
    if (it != group_states.end()) {
        if (it->second.userinstalled) {
            group_reason = transaction::TransactionItemReason::USER;
        } else {
            group_reason = transaction::TransactionItemReason::DEPENDENCY;
        }
    }

    return group_reason;
}

transaction::TransactionItemReason State::get_environment_reason(const std::string & id) {
    transaction::TransactionItemReason environment_reason = transaction::TransactionItemReason::NONE;
    auto it = environment_states.find(id);
    if (it != environment_states.end()) {
        environment_reason = transaction::TransactionItemReason::USER;
    }

    return environment_reason;
}


void State::set_package_reason(const std::string & na, transaction::TransactionItemReason reason) {
    auto reason_str = transaction::transaction_item_reason_to_string(reason);

    libdnf_assert(
        (reason == transaction::TransactionItemReason::WEAK_DEPENDENCY ||
         reason == transaction::TransactionItemReason::DEPENDENCY ||
         reason == transaction::TransactionItemReason::USER ||
         reason == transaction::TransactionItemReason::EXTERNAL_USER),
        "Unexpected system state package reason: \"{}\"",
        reason_str);

    package_states[na].reason = reason_str;
}


std::set<std::string> State::get_packages_by_reason(const std::set<transaction::TransactionItemReason> & reasons) {
    std::set<std::string> packages;
    if (reasons.contains(transaction::TransactionItemReason::GROUP)) {
        auto & package_groups = get_package_groups_cache();
        for (const auto & pkg : package_groups) {
            packages.emplace(pkg.first);
        }
    }
    std::set<std::string> reasons_str;
    for (const auto & reason : reasons) {
        reasons_str.emplace(transaction::transaction_item_reason_to_string(reason));
    }
    for (const auto & [na, pkg_state] : package_states) {
        if (reasons_str.contains(pkg_state.reason)) {
            packages.emplace(na);
        }
    }
    return packages;
}


void State::remove_package_na_state(const std::string & na) {
    package_states.erase(na);
}


std::string State::get_package_from_repo(const std::string & nevra) {
    auto it = nevra_states.find(nevra);
    if (it == nevra_states.end()) {
        throw StateNotFoundError("NEVRA", nevra);
    }

    return it->second.from_repo;
}


void State::set_package_from_repo(const std::string & nevra, const std::string & from_repo) {
    nevra_states[nevra].from_repo = from_repo;
}


void State::remove_package_nevra_state(const std::string & nevra) {
    nevra_states.erase(nevra);
}


GroupState State::get_group_state(const std::string & id) {
    auto it = group_states.find(id);
    if (it == group_states.end()) {
        throw StateNotFoundError("Group", id);
    }

    return it->second;
}


void State::set_group_state(const std::string & id, const GroupState & group_state) {
    group_states[id] = group_state;
    package_groups_cache.reset();
}


void State::remove_group_state(const std::string & id) {
    group_states.erase(id);
    package_groups_cache.reset();
}


EnvironmentState State::get_environment_state(const std::string & id) {
    auto it = environment_states.find(id);
    if (it == environment_states.end()) {
        throw StateNotFoundError("Environment", id);
    }

    return it->second;
}


void State::set_environment_state(const std::string & id, const EnvironmentState & environment_state) {
    environment_states[id] = environment_state;
}


void State::remove_environment_state(const std::string & id) {
    environment_states.erase(id);
}


std::set<std::string> State::get_package_groups(const std::string & name) {
    auto & package_groups = get_package_groups_cache();
    auto it = package_groups.find(name);
    if (it == package_groups.end()) {
        return {};
    } else {
        return it->second;
    }
}


std::vector<std::string> State::get_installed_groups() {
    std::vector<std::string> group_ids;
    group_ids.reserve(group_states.size());
    for (const auto & grp : group_states) {
        group_ids.push_back(grp.first);
    }
    return group_ids;
}


std::vector<std::string> State::get_installed_environments() {
    std::vector<std::string> environment_ids;
    environment_ids.reserve(environment_states.size());
    for (const auto & env : environment_states) {
        environment_ids.push_back(env.first);
    }
    return environment_ids;
}


std::set<std::string> State::get_group_environments(const std::string & id) {
    std::set<std::string> environments;
    for (const auto & env_iter : environment_states) {
        auto & env = env_iter.second;
        if (std::find(env.groups.begin(), env.groups.end(), id) != env.groups.end()) {
            environments.emplace(env_iter.first);
        }
    }
    return environments;
}

#ifdef WITH_MODULEMD
const std::map<std::string, ModuleState> & State::get_module_states() {
    return module_states;
}


ModuleState State::get_module_state(const std::string & name) {
    auto it = module_states.find(name);
    if (it == module_states.end()) {
        throw StateNotFoundError("Module", name);
    }

    return it->second;
}


void State::set_module_state(const std::string & name, const ModuleState & module_state) {
    module_states[name] = module_state;
}


void State::remove_module_state(const std::string & name) {
    module_states.erase(name);
}
#endif


std::string State::get_rpmdb_cookie() const {
    return system_state.rpmdb_cookie;
}


void State::set_rpmdb_cookie(const std::string & cookie) {
    system_state.rpmdb_cookie = cookie;
}


#ifdef TOML11_COMPAT
template <typename T>
static toml::value make_top_value(const std::string & key, const T & value) {
    return toml::value({{key, value}, { "version", make_version() }});
}


static std::string toml_format(const toml::value & value) {
    return toml::format<toml::discard_comments, std::map, std::vector>(value);
}
#else
template <typename T>
static toml::ordered_value make_top_value(const std::string & key, const T & value) {
    return toml::ordered_value({{"version", make_version()}, {key, toml::ordered_value(value)}});
}


static std::string toml_format(const toml::ordered_value & value) {
    return toml::format(value);
}
#endif  // #ifdef TOML11_COMPAT


static std::filesystem::path suffix_new(std::filesystem::path p) {
    p += ".new";
    return p;
}


static void remove_new_suffix(std::filesystem::path p, bool skip_missing) {
    std::filesystem::path p_target = p;
    auto p_new = suffix_new(p);

    if (std::filesystem::exists(p_new) || !skip_missing) {
        utils::fs::move_recursive(p_new, p_target);
    }
}


void State::rename_new_system_state_files(bool skip_missing) {
    // package state file has to be always renamed first, logic in load() relies on this
    remove_new_suffix(get_package_state_path(), skip_missing);

    remove_new_suffix(get_nevra_state_path(), skip_missing);
    remove_new_suffix(get_group_state_path(), skip_missing);
    remove_new_suffix(get_environment_state_path(), skip_missing);
    remove_new_suffix(get_system_state_path(), skip_missing);
#ifdef WITH_MODULEMD
    remove_new_suffix(get_module_state_path(), skip_missing);
#endif
}


void State::save() {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec) {
        throw FileSystemError(errno, path, M_("{}"), ec.message());
    }

    utils::fs::File(suffix_new(get_package_state_path()), "w")
        .write(toml_format(make_top_value("packages", package_states)));
    utils::fs::File(suffix_new(get_nevra_state_path()), "w").write(toml_format(make_top_value("nevras", nevra_states)));
    utils::fs::File(suffix_new(get_group_state_path()), "w").write(toml_format(make_top_value("groups", group_states)));
    utils::fs::File(suffix_new(get_environment_state_path()), "w")
        .write(toml_format(make_top_value("environments", environment_states)));
#ifdef WITH_MODULEMD
    utils::fs::File(suffix_new(get_module_state_path()), "w")
        .write(toml_format(make_top_value("modules", module_states)));
#endif
    utils::fs::File(suffix_new(get_system_state_path()), "w")
        .write(toml_format(make_top_value("system", system_state)));

    // Once all new files were written replace the current files
    rename_new_system_state_files(false);
}


template <typename T>
static T load_toml_data(const std::string & path, const std::string & key) {
    if (!std::filesystem::exists(path)) {
        // TODO(lukash) log this?
        return {};
    }

    const auto toml_value = toml::parse(path);

    auto version_string = toml::find<std::string>(toml_value, "version");
    auto version_parsed = parse_version(version_string);

    if (version_parsed.first != version_major || version_parsed.second > version_minor) {
        throw UnsupportedVersionError(
            M_("Unsupported TOML version \"{}\", maximum supported version is \"{}\""), version_string, make_version());
    }

    // TODO(lukash) throws std::runtime_error with no error description in case opening the file fails
    return toml::find<T>(toml_value, key);
}


// In a given directory find all files endig with ".new" suffix and return them
static std::vector<std::filesystem::path> gather_suffix_new_files(const std::filesystem::path & directory) {
    std::vector<std::filesystem::path> new_files;
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return new_files;
    }

    for (const auto & entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".new") {
            new_files.push_back(entry.path());
        }
    }
    return new_files;
}


void State::load() {
    std::string path;
    try {
        auto new_suffix_files = gather_suffix_new_files(this->path);
        if (!new_suffix_files.empty()) {
            auto logger = base->get_logger();
            logger->warning("System state: unfinished update found");

            try {
                // package_state new file is renamed first, if it exists the .new system state
                // files are likely incomplete and we cannot use them
                if (std::filesystem::exists(suffix_new(get_package_state_path()))) {
                    // TODO(amatej): Once https://github.com/rpm-software-management/dnf5/issues/1610 is done we should
                    //               suggest to rebuild the system state, some information is likely missing because
                    //               system state update happens after the transaction is finished.
                    logger->error("System state: cannot use partially written system state files");
                    for (const auto & f : new_suffix_files) {
                        std::filesystem::remove(f);
                    }
                } else {
                    logger->warning(
                        "System state: update interruption happened during renaming which means all the new system "
                        "state files were written, using them.");
                    rename_new_system_state_files(true);
                }
            } catch (const std::filesystem::filesystem_error & ex) {
                logger->error("System state: cannot recover: ", ex.what());
            }
        }

        path = get_package_state_path();
        package_states = load_toml_data<std::map<std::string, PackageState>>(path, "packages");
        path = get_nevra_state_path();
        nevra_states = load_toml_data<std::map<std::string, NevraState>>(path, "nevras");
        path = get_group_state_path();
        group_states = load_toml_data<std::map<std::string, GroupState>>(path, "groups");
        path = get_environment_state_path();
        environment_states = load_toml_data<std::map<std::string, EnvironmentState>>(path, "environments");
#ifdef WITH_MODULEMD
        path = get_module_state_path();
        module_states = load_toml_data<std::map<std::string, ModuleState>>(path, "modules");
#endif
        path = get_system_state_path();
        system_state = load_toml_data<SystemState>(path, "system");
    } catch (const InvalidVersionError & ex) {
        throw;
    } catch (const UnsupportedVersionError & ex) {
        throw;
    } catch (const std::exception & ex) {
        throw StateLoadError(path, ex.what());
    }
    package_groups_cache.reset();
}

const std::map<std::string, std::set<std::string>> & State::get_package_groups_cache() {
    if (!package_groups_cache) {
        std::map<std::string, std::set<std::string>> cache;
        for (const auto & [group_id, group_state] : group_states) {
            for (const auto & pkg : group_state.packages) {
                cache[pkg].emplace(group_id);
            }
        }
        package_groups_cache.emplace(std::move(cache));
    }
    return package_groups_cache.value();
}


std::filesystem::path State::get_group_xml_dir() {
    return path / "comps_groups";
}


std::filesystem::path State::get_package_state_path() {
    return path / "packages.toml";
}


std::filesystem::path State::get_nevra_state_path() {
    return path / "nevras.toml";
}


std::filesystem::path State::get_group_state_path() {
    return path / "groups.toml";
}


std::filesystem::path State::get_environment_state_path() {
    return path / "environments.toml";
}


std::filesystem::path State::get_module_state_path() {
    return path / "modules.toml";
}


std::filesystem::path State::get_system_state_path() {
    return path / "system.toml";
}

void State::reset_packages_states(
    std::map<std::string, libdnf5::system::PackageState> && package_states,
    std::map<std::string, libdnf5::system::NevraState> && nevra_states,
    std::map<std::string, libdnf5::system::GroupState> && group_states,
    std::map<std::string, libdnf5::system::EnvironmentState> && environment_states) {
    this->package_states = std::move(package_states);
    this->nevra_states = std::move(nevra_states);
    this->group_states = std::move(group_states);
    this->environment_states = std::move(environment_states);

    // Try to save the new system state.
    // dnf can be used without root privileges or with read-only system state location.
    // In that case ignore the filesystem errors and only keep new system state in memory.
    try {
        save();
    } catch (const FileSystemError & e) {
        // TODO(mblaha) - log this? (will need access to the base)
    }
}

}  // namespace libdnf5::system
