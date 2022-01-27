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

#include <map>


// map string package attribute name to actual attribute
const std::map<std::string, PackageAttribute> package_attributes{
    {"name", PackageAttribute::name},
    {"epoch", PackageAttribute::epoch},
    {"version", PackageAttribute::version},
    {"release", PackageAttribute::release},
    {"arch", PackageAttribute::arch},
    {"repo", PackageAttribute::repo},
    {"is_installed", PackageAttribute::is_installed},
    {"install_size", PackageAttribute::install_size},
    {"package_size", PackageAttribute::package_size},
    {"sourcerpm", PackageAttribute::sourcerpm},
    {"summary", PackageAttribute::summary},
    {"url", PackageAttribute::url},
    {"license", PackageAttribute::license},
    {"description", PackageAttribute::description},
    {"files", PackageAttribute::files},
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
    {"full_nevra", PackageAttribute::full_nevra}};

std::vector<std::string> reldeplist_to_strings(const libdnf::rpm::ReldepList & reldeps) {
    std::vector<std::string> lst;
    for (auto reldep : reldeps) {
        lst.emplace_back(reldep.to_string());
    }
    return lst;
}

dnfdaemon::KeyValueMap package_to_map(
    const libdnf::rpm::Package & libdnf_package, const std::vector<std::string> & attributes) {
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
            case PackageAttribute::repo:
                dbus_package.emplace(attr, libdnf_package.get_repo_id());
                break;
            case PackageAttribute::is_installed:
                dbus_package.emplace(attr, libdnf_package.is_installed());
                break;
            case PackageAttribute::install_size:
                dbus_package.emplace(attr, static_cast<uint64_t>(libdnf_package.get_install_size()));
                break;
            case PackageAttribute::package_size:
                dbus_package.emplace(attr, static_cast<uint64_t>(libdnf_package.get_package_size()));
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
            case PackageAttribute::provides:
                dbus_package.emplace(attr, reldeplist_to_strings(libdnf_package.get_provides()));
                break;
            case PackageAttribute::files:
                dbus_package.emplace(attr, libdnf_package.get_files());
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
        }
    }
    return dbus_package;
}
