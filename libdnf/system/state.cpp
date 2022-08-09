/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/system/state.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/fs/file.hpp"
#include "utils/string.hpp"

#include <toml.hpp>


namespace toml {

template <>
struct from<libdnf::system::PackageState> {
    static libdnf::system::PackageState from_toml(const value & v) {
        libdnf::system::PackageState pkg_state;

        pkg_state.reason = toml::find<std::string>(v, "reason");

        return pkg_state;
    }
};


template <>
struct into<libdnf::system::PackageState> {
    static toml::value into_toml(const libdnf::system::PackageState & pkg_state) {
        toml::value res;

        res["reason"] = pkg_state.reason;

        return res;
    }
};


template <>
struct from<libdnf::system::NevraState> {
    static libdnf::system::NevraState from_toml(const value & v) {
        libdnf::system::NevraState nevra_state;

        if (v.contains("from_repo")) {
            nevra_state.from_repo = toml::find<std::string>(v, "from_repo");
        }

        return nevra_state;
    }
};


template <>
struct into<libdnf::system::NevraState> {
    static toml::value into_toml(const libdnf::system::NevraState & nevra_state) {
        toml::value res;

        res["from_repo"] = nevra_state.from_repo;

        return res;
    }
};


template <>
struct from<libdnf::system::GroupState> {
    static libdnf::system::GroupState from_toml(const value & v) {
        libdnf::system::GroupState group_state;

        group_state.userinstalled = toml::find<bool>(v, "userinstalled");
        if (v.contains("packages")) {
            group_state.packages = toml::find<std::vector<std::string>>(v, "packages");
        }

        return group_state;
    }
};


template <>
struct into<libdnf::system::GroupState> {
    static toml::value into_toml(const libdnf::system::GroupState & group_state) {
        toml::value res;

        res["userinstalled"] = group_state.userinstalled;
        res["packages"] = group_state.packages;

        return res;
    }
};


template <>
struct from<libdnf::system::ModuleState> {
    static libdnf::system::ModuleState from_toml(const value & v) {
        libdnf::system::ModuleState module_state;

        module_state.enabled_stream = toml::find<std::string>(v, "enabled_stream");
        module_state.state = libdnf::module::module_state_from_string(toml::find<std::string>(v, "state"));
        module_state.installed_profiles = toml::find<std::vector<std::string>>(v, "installed_profiles");

        return module_state;
    }
};


template <>
struct into<libdnf::system::ModuleState> {
    static toml::value into_toml(const libdnf::system::ModuleState & module_state) {
        toml::value res;

        res["enabled_stream"] = module_state.enabled_stream;
        res["state"] = libdnf::module::module_state_to_string(module_state.state);
        res["installed_profiles"] = module_state.installed_profiles;

        return res;
    }
};

}  // namespace toml


namespace libdnf::system {

const constexpr uint8_t version_major{1};
const constexpr uint8_t version_minor{0};


static std::string make_version() {
    return libdnf::utils::sformat("{}.{}", version_major, version_minor);
}


static std::pair<uint64_t, uint64_t> parse_version(const std::string & version) {
    std::vector<std::string> split_version = libdnf::utils::string::split(version, ".");
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
    : libdnf::Error(M_("{} state for \"{}\" not found."), type, key) {}


State::State(const std::filesystem::path & path) : path(path) {
    load();
}


transaction::TransactionItemReason State::get_package_reason(const std::string & na) {
    auto it = package_states.find(na);
    if (it == package_states.end()) {
        return transaction::TransactionItemReason::NONE;
    }

    // TODO(lukash) this allows more reasons than valid here, assert?
    return transaction::transaction_item_reason_from_string(it->second.reason);
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
}


void State::remove_group_state(const std::string & id) {
    group_states.erase(id);
}


std::string State::get_module_enabled_stream(const std::string & name) {
    auto it = module_states.find(name);
    if (it == module_states.end()) {
        throw StateNotFoundError("Module", name);
    }

    return it->second.enabled_stream;
}


void State::set_module_enabled_stream(const std::string & name, const std::string & stream) {
    module_states[name].enabled_stream = stream;
}


module::ModuleState State::get_module_state(const std::string & name) {
    auto it = module_states.find(name);
    if (it == module_states.end()) {
        throw StateNotFoundError("Module", name);
    }

    return it->second.state;
}


void State::set_module_state(const std::string & name, module::ModuleState state) {
    module_states[name].state = state;
}


std::vector<std::string> State::get_module_installed_profiles(const std::string & name) {
    auto it = module_states.find(name);
    if (it == module_states.end()) {
        throw StateNotFoundError("Module", name);
    }

    return it->second.installed_profiles;
}


void State::set_module_installed_profiles(const std::string & name, const std::vector<std::string> & profiles) {
    module_states[name].installed_profiles = profiles;
}


void State::remove_module_state(const std::string & name) {
    module_states.erase(name);
}


template <typename T>
static toml::value make_top_value(const std::string & key, const T & value) {
    return toml::value({{key, value}, {"version", make_version()}});
}


void State::save() {
    std::filesystem::create_directories(path);

    utils::fs::File(get_package_state_path(), "w").write(toml::format(make_top_value("packages", package_states)));
    utils::fs::File(get_nevra_state_path(), "w").write(toml::format(make_top_value("nevras", nevra_states)));
    utils::fs::File(get_group_state_path(), "w").write(toml::format(make_top_value("groups", group_states)));
    utils::fs::File(get_module_state_path(), "w").write(toml::format(make_top_value("modules", module_states)));
}


template <typename T>
static std::map<std::string, T> load_toml_to_map(const std::string & path, const std::string & key) {
    if (!std::filesystem::exists(path)) {
        // TODO(lukash) log this?
        return {};
    }

    auto toml_value = toml::parse(path);

    auto version_string = toml::find<std::string>(toml_value, "version");
    auto version_parsed = parse_version(version_string);

    if (version_parsed.first != version_major || version_parsed.second > version_minor) {
        throw UnsupportedVersionError(
            M_("Unsupported TOML version \"{}\", maximum supported version is \"{}\""), version_string, make_version());
    }

    // TODO(lukash) throws std::runtime_error with no error description in case opening the file fails
    return toml::find<std::map<std::string, T>>(toml_value, key);
}


void State::load() {
    package_states = load_toml_to_map<PackageState>(get_package_state_path(), "packages");
    nevra_states = load_toml_to_map<NevraState>(get_nevra_state_path(), "nevras");
    group_states = load_toml_to_map<GroupState>(get_group_state_path(), "groups");
    module_states = load_toml_to_map<ModuleState>(get_module_state_path(), "modules");
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


std::filesystem::path State::get_module_state_path() {
    return path / "modules.toml";
}

}  // namespace libdnf::system
