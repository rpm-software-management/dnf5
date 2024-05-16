/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "package.hpp"

#include <fmt/format.h>
#include <json-c/json.h>

#include <map>

// map string package attribute name to actual attribute
const std::map<std::string, PackageAttribute> package_attributes{
    {"name", PackageAttribute::name},
    {"epoch", PackageAttribute::epoch},
    {"version", PackageAttribute::version},
    {"release", PackageAttribute::release},
    {"arch", PackageAttribute::arch},
    {"repo_id", PackageAttribute::repo_id},
    {"from_repo_id", PackageAttribute::from_repo_id},
    {"is_installed", PackageAttribute::is_installed},
    {"install_size", PackageAttribute::install_size},
    {"download_size", PackageAttribute::download_size},
    {"buildtime", PackageAttribute::buildtime},
    {"sourcerpm", PackageAttribute::sourcerpm},
    {"summary", PackageAttribute::summary},
    {"url", PackageAttribute::url},
    {"license", PackageAttribute::license},
    {"description", PackageAttribute::description},
    {"files", PackageAttribute::files},
    {"changelogs", PackageAttribute::changelogs},
    {"provides", PackageAttribute::provides},
    {"requires", PackageAttribute::requires_all},
    {"requires_pre", PackageAttribute::requires_pre},
    {"conflicts", PackageAttribute::conflicts},
    {"obsoletes", PackageAttribute::obsoletes},
    {"recommends", PackageAttribute::recommends},
    {"suggests", PackageAttribute::suggests},
    {"enhances", PackageAttribute::enhances},
    {"supplements", PackageAttribute::supplements},
    {"evr", PackageAttribute::evr},
    {"nevra", PackageAttribute::nevra},
    {"full_nevra", PackageAttribute::full_nevra},
    {"reason", PackageAttribute::reason},
    {"vendor", PackageAttribute::vendor},
    {"group", PackageAttribute::group}};

static std::vector<std::string> reldeplist_to_strings(const libdnf5::rpm::ReldepList & reldeps) {
    std::vector<std::string> lst;
    for (auto reldep : reldeps) {
        lst.emplace_back(reldep.to_string());
    }
    return lst;
}

static std::vector<dnfdaemon::Changelog> changelogs_to_list(const libdnf5::rpm::Package & libdnf_package) {
    std::vector<dnfdaemon::Changelog> changelogs;

    for (const auto & chlog : libdnf_package.get_changelogs()) {
        changelogs.emplace_back(static_cast<int64_t>(chlog.get_timestamp()), chlog.get_author(), chlog.get_text());
    }

    return changelogs;
}

dnfdaemon::KeyValueMap package_to_map(
    const libdnf5::rpm::Package & libdnf_package, const std::vector<std::string> & attributes) {
    dnfdaemon::KeyValueMap dbus_package;
    // add package id by default
    dbus_package.emplace(std::make_pair("id", libdnf_package.get_id().id));
    // attributes required by client
    for (auto & attr : attributes) {
        auto it = package_attributes.find(attr);
        if (it == package_attributes.end()) {
            throw std::runtime_error(fmt::format("Package attribute '{}' not supported", attr));
        }
        switch (it->second) {
            case PackageAttribute::name:
                dbus_package.emplace(attr, libdnf_package.get_name());
                break;
            case PackageAttribute::epoch:
                dbus_package.emplace(attr, libdnf_package.get_epoch());
                break;
            case PackageAttribute::version:
                dbus_package.emplace(attr, libdnf_package.get_version());
                break;
            case PackageAttribute::release:
                dbus_package.emplace(attr, libdnf_package.get_release());
                break;
            case PackageAttribute::arch:
                dbus_package.emplace(attr, libdnf_package.get_arch());
                break;
            case PackageAttribute::repo_id:
                dbus_package.emplace(attr, libdnf_package.get_repo_id());
                break;
            case PackageAttribute::from_repo_id:
                dbus_package.emplace(attr, libdnf_package.get_from_repo_id());
                break;
            case PackageAttribute::is_installed:
                dbus_package.emplace(attr, libdnf_package.is_installed());
                break;
            case PackageAttribute::install_size:
                dbus_package.emplace(attr, static_cast<uint64_t>(libdnf_package.get_install_size()));
                break;
            case PackageAttribute::download_size:
                dbus_package.emplace(attr, static_cast<uint64_t>(libdnf_package.get_download_size()));
                break;
            case PackageAttribute::buildtime:
                dbus_package.emplace(attr, static_cast<uint64_t>(libdnf_package.get_build_time()));
                break;
            case PackageAttribute::sourcerpm:
                dbus_package.emplace(attr, libdnf_package.get_sourcerpm());
                break;
            case PackageAttribute::summary:
                dbus_package.emplace(attr, libdnf_package.get_summary());
                break;
            case PackageAttribute::url:
                dbus_package.emplace(attr, libdnf_package.get_url());
                break;
            case PackageAttribute::license:
                dbus_package.emplace(attr, libdnf_package.get_license());
                break;
            case PackageAttribute::description:
                dbus_package.emplace(attr, libdnf_package.get_description());
                break;
            case PackageAttribute::files:
                dbus_package.emplace(attr, libdnf_package.get_files());
                break;
            case PackageAttribute::changelogs:
                dbus_package.emplace(attr, changelogs_to_list(libdnf_package));
                break;
            case PackageAttribute::provides:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_provides()));
                break;
            case PackageAttribute::requires_all:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_requires()));
                break;
            case PackageAttribute::requires_pre:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_requires_pre()));
                break;
            case PackageAttribute::prereq_ignoreinst:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_prereq_ignoreinst()));
                break;
            case PackageAttribute::regular_requires:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_regular_requires()));
                break;
            case PackageAttribute::conflicts:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_conflicts()));
                break;
            case PackageAttribute::obsoletes:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_obsoletes()));
                break;
            case PackageAttribute::recommends:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_recommends()));
                break;
            case PackageAttribute::suggests:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_suggests()));
                break;
            case PackageAttribute::enhances:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_enhances()));
                break;
            case PackageAttribute::supplements:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_supplements()));
                break;
            case PackageAttribute::evr:
                dbus_package.emplace(attr, libdnf_package.get_evr());
                break;
            case PackageAttribute::nevra:
                dbus_package.emplace(attr, libdnf_package.get_nevra());
                break;
            case PackageAttribute::full_nevra:
                dbus_package.emplace(attr, libdnf_package.get_full_nevra());
                break;
            case PackageAttribute::reason:
                dbus_package.emplace(
                    attr, libdnf5::transaction::transaction_item_reason_to_string(libdnf_package.get_reason()));
                break;
            case PackageAttribute::vendor:
                dbus_package.emplace(attr, libdnf_package.get_vendor());
                break;
            case PackageAttribute::group:
                dbus_package.emplace(attr, libdnf_package.get_group());
                break;
        }
    }
    return dbus_package;
}

static void add_string_list(json_object * json_pkg, const char * cattr, const std::vector<std::string> & vector) {
    json_object * array = json_object_new_array();
    json_object_object_add(json_pkg, cattr, array);
    for (const auto & elem : vector) {
        json_object_array_add(array, json_object_new_string(elem.c_str()));
    }
}

std::string package_to_json(const libdnf5::rpm::Package & libdnf_package, const std::vector<std::string> & attributes) {
    json_object * json_pkg = json_object_new_object();

    // add package id by default
    json_object_object_add(json_pkg, "id", json_object_new_int(libdnf_package.get_id().id));

    // attributes required by client
    for (const auto & attr : attributes) {
        const auto * const cattr = attr.c_str();
        auto it = package_attributes.find(attr);
        if (it == package_attributes.end()) {
            throw std::runtime_error(fmt::format("Package attribute '{}' not supported", attr));
        }
        switch (it->second) {
            case PackageAttribute::name:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_name().c_str()));

                break;
            case PackageAttribute::epoch:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_epoch().c_str()));
                break;
            case PackageAttribute::version:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_version().c_str()));
                break;
            case PackageAttribute::release:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_release().c_str()));
                break;
            case PackageAttribute::arch:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_arch().c_str()));
                break;
            case PackageAttribute::repo_id:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_repo_id().c_str()));
                break;
            case PackageAttribute::from_repo_id:
                json_object_object_add(
                    json_pkg, cattr, json_object_new_string(libdnf_package.get_from_repo_id().c_str()));
                break;
            case PackageAttribute::is_installed:
                json_object_object_add(
                    json_pkg, cattr, json_object_new_boolean(static_cast<json_bool>(libdnf_package.is_installed())));
                break;
            case PackageAttribute::install_size:
                json_object_object_add(
                    json_pkg, cattr, json_object_new_int64(static_cast<int64_t>(libdnf_package.get_install_size())));
                break;
            case PackageAttribute::download_size:
                json_object_object_add(
                    json_pkg, cattr, json_object_new_int64(static_cast<int64_t>(libdnf_package.get_download_size())));
                break;
            case PackageAttribute::buildtime:
                json_object_object_add(
                    json_pkg, cattr, json_object_new_int64(static_cast<int64_t>(libdnf_package.get_build_time())));
                break;
            case PackageAttribute::sourcerpm:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_sourcerpm().c_str()));
                break;
            case PackageAttribute::summary:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_summary().c_str()));
                break;
            case PackageAttribute::url:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_url().c_str()));
                break;
            case PackageAttribute::license:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_license().c_str()));
                break;
            case PackageAttribute::description:
                json_object_object_add(
                    json_pkg, cattr, json_object_new_string(libdnf_package.get_description().c_str()));
                break;
            case PackageAttribute::files:
                add_string_list(json_pkg, cattr, libdnf_package.get_files());
                break;
            case PackageAttribute::changelogs: {
                json_object * array = json_object_new_array();
                json_object_object_add(json_pkg, cattr, array);
                for (const auto & libdnf_chlog : libdnf_package.get_changelogs()) {
                    json_object * chlog = json_object_new_array();
                    json_object_array_add(
                        chlog, json_object_new_int64(static_cast<int64_t>(libdnf_chlog.get_timestamp())));
                    json_object_array_add(chlog, json_object_new_string(libdnf_chlog.get_author().c_str()));
                    json_object_array_add(chlog, json_object_new_string(libdnf_chlog.get_text().c_str()));

                    json_object_array_add(array, chlog);
                }
                break;
            }
            case PackageAttribute::provides:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_provides()));
                break;
            case PackageAttribute::requires_all:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_requires()));
                break;
            case PackageAttribute::requires_pre:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_requires_pre()));
                break;
            case PackageAttribute::prereq_ignoreinst:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_prereq_ignoreinst()));
                break;
            case PackageAttribute::regular_requires:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_regular_requires()));
                break;
            case PackageAttribute::conflicts:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_conflicts()));
                break;
            case PackageAttribute::obsoletes:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_obsoletes()));
                break;
            case PackageAttribute::recommends:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_recommends()));
                break;
            case PackageAttribute::suggests:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_suggests()));
                break;
            case PackageAttribute::enhances:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_enhances()));
                break;
            case PackageAttribute::supplements:
                add_string_list(json_pkg, cattr, reldeplist_to_strings(libdnf_package.get_supplements()));
                break;
            case PackageAttribute::evr:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_evr().c_str()));
                break;
            case PackageAttribute::nevra:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_nevra().c_str()));
                break;
            case PackageAttribute::full_nevra:
                json_object_object_add(
                    json_pkg, cattr, json_object_new_string(libdnf_package.get_full_nevra().c_str()));
                break;
            case PackageAttribute::reason:
                json_object_object_add(
                    json_pkg,
                    cattr,
                    json_object_new_string(
                        libdnf5::transaction::transaction_item_reason_to_string(libdnf_package.get_reason()).c_str()));
                break;
            case PackageAttribute::vendor:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_vendor().c_str()));
                break;
            case PackageAttribute::group:
                json_object_object_add(json_pkg, cattr, json_object_new_string(libdnf_package.get_group().c_str()));
                break;
        }
    }

    // do not add any extra white spaces, make it one-liner json
    std::string res = json_object_to_json_string_ext(json_pkg, JSON_C_TO_STRING_PLAIN);
    json_object_put(json_pkg);
    return res;
}
