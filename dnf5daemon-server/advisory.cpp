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

#include "advisory.hpp"

#include <fmt/format.h>
#include <libdnf5/advisory/advisory_collection.hpp>
#include <libdnf5/advisory/advisory_reference.hpp>
#include <libdnf5/rpm/nevra.hpp>

#include <map>

namespace dnfdaemon {

// map string advisory attribute name to actual attribute
const std::map<std::string, AdvisoryAttribute> advisory_attributes{
    {"advisoryid", AdvisoryAttribute::advisoryid},
    {"name", AdvisoryAttribute::name},
    {"severity", AdvisoryAttribute::severity},
    {"type", AdvisoryAttribute::type},
    {"buildtime", AdvisoryAttribute::buildtime},
    {"vendor", AdvisoryAttribute::vendor},
    {"description", AdvisoryAttribute::description},
    {"title", AdvisoryAttribute::title},
    {"status", AdvisoryAttribute::status},
    {"rights", AdvisoryAttribute::rights},
    {"message", AdvisoryAttribute::message},
    {"references", AdvisoryAttribute::references},
    {"collections", AdvisoryAttribute::collections},
};

std::vector<AdvisoryReference> references_to_list(const libdnf5::advisory::Advisory & libdnf_advisory) {
    std::vector<AdvisoryReference> references;

    for (const auto & ref : libdnf_advisory.get_references()) {
        references.emplace_back(ref.get_id(), ref.get_type(), ref.get_title(), ref.get_url());
    }

    return references;
}

KeyValueMapList collections_to_list(
    const libdnf5::advisory::Advisory & libdnf_advisory,
    const std::unordered_map<std::string, libdnf5::rpm::Package> & installed_versions) {
    KeyValueMapList collections;
    for (auto & col : libdnf_advisory.get_collections()) {
        KeyValueMapList packages;
        auto libdnf_packages = col.get_packages();
        std::sort(
            libdnf_packages.begin(),
            libdnf_packages.end(),
            libdnf5::rpm::cmp_nevra<libdnf5::advisory::AdvisoryPackage>);
        for (const auto & pkg : libdnf_packages) {
            KeyValueMap package;
            auto name = pkg.get_name();
            auto arch = pkg.get_arch();

            package["n"] = name;
            package["e"] = pkg.get_epoch();
            package["v"] = pkg.get_version();
            package["r"] = pkg.get_release();
            package["a"] = arch;
            package["nevra"] = pkg.get_nevra();

            std::string na{std::move(name)};
            na.append(".");
            na.append(arch);
            auto it = installed_versions.find(na);
            if (it == installed_versions.end()) {
                // advisory package is not installed => not related to system
                package["applicability"] = "unrelated";
            } else if (libdnf5::rpm::evrcmp(it->second, pkg) < 0) {
                // installed version is lower than one in advisory
                package["applicability"] = "available";
            } else {
                package["applicability"] = "installed";
            }

            packages.emplace_back(std::move(package));
        }

        KeyValueMapList modules;
        auto libdnf_modules = col.get_modules();
        for (const auto & mdl : libdnf_modules) {
            KeyValueMap col_module;
            col_module["n"] = mdl.get_name();
            col_module["s"] = mdl.get_stream();
            col_module["v"] = mdl.get_version();
            col_module["c"] = mdl.get_context();
            col_module["a"] = mdl.get_arch();
            col_module["nsvca"] = mdl.get_nsvca();
            modules.emplace_back(std::move(col_module));
        }

        KeyValueMap collection;
        collection["packages"] = std::move(packages);
        collection["modules"] = std::move(modules);
        collections.emplace_back(std::move(collection));
    }
    return collections;
}

KeyValueMap advisory_to_map(
    const libdnf5::advisory::Advisory & libdnf_advisory,
    const std::vector<std::string> & attributes,
    const std::unordered_map<std::string, libdnf5::rpm::Package> & installed_versions) {
    KeyValueMap dbus_advisory;
    // add advisory id by default
    dbus_advisory.emplace("advisoryid", libdnf_advisory.get_id().id);
    // attributes required by client
    for (auto & attr : attributes) {
        auto it = advisory_attributes.find(attr);
        if (it == advisory_attributes.end()) {
            throw std::runtime_error(fmt::format("Advisory attribute '{}' not supported", attr));
        }
        switch (it->second) {
            case AdvisoryAttribute::advisoryid:
                // already added by default
                break;
            case AdvisoryAttribute::name:
                dbus_advisory.emplace(attr, libdnf_advisory.get_name());
                break;
            case AdvisoryAttribute::severity:
                dbus_advisory.emplace(attr, libdnf_advisory.get_severity());
                break;
            case AdvisoryAttribute::type:
                dbus_advisory.emplace(attr, libdnf_advisory.get_type());
                break;
            case AdvisoryAttribute::buildtime:
                dbus_advisory.emplace(attr, static_cast<uint64_t>(libdnf_advisory.get_buildtime()));
                break;
            case AdvisoryAttribute::vendor:
                dbus_advisory.emplace(attr, libdnf_advisory.get_vendor());
                break;
            case AdvisoryAttribute::description:
                dbus_advisory.emplace(attr, libdnf_advisory.get_description());
                break;
            case AdvisoryAttribute::title:
                dbus_advisory.emplace(attr, libdnf_advisory.get_title());
                break;
            case AdvisoryAttribute::status:
                dbus_advisory.emplace(attr, libdnf_advisory.get_status());
                break;
            case AdvisoryAttribute::rights:
                dbus_advisory.emplace(attr, libdnf_advisory.get_rights());
                break;
            case AdvisoryAttribute::message:
                dbus_advisory.emplace(attr, libdnf_advisory.get_message());
                break;
            case AdvisoryAttribute::collections:
                dbus_advisory.emplace(attr, collections_to_list(libdnf_advisory, installed_versions));
                break;
            case AdvisoryAttribute::references:
                dbus_advisory.emplace(attr, references_to_list(libdnf_advisory));
                break;
        }
    }
    return dbus_advisory;
}

}  // namespace dnfdaemon
