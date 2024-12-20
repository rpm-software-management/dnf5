/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf5-cli/output/advisoryinfo.hpp"

#include "key_value_table.hpp"
#include "utils/string.hpp"

#include <json.h>

#include <iostream>

namespace libdnf5::cli::output {

class AdvisoryInfo::Impl : public KeyValueTable {
public:
    void add_advisory(IAdvisory & advisory);
};

class AdvisoryInfoJSON::Impl {
public:
    Impl() { json_advisories = json_object_new_object(); };
    ~Impl() { json_object_put(json_advisories); };
    void add_advisory(IAdvisory & advisory);
    void print() {
        std::cout << json_object_to_json_string_ext(json_advisories, JSON_C_TO_STRING_PRETTY) << std::endl;
    };
    json_object * json_advisories;
};


void AdvisoryInfo::Impl::add_advisory(IAdvisory & advisory) {
    add_line("Name", advisory.get_name(), "bold");
    add_line("Title", advisory.get_title());
    add_line("Severity", advisory.get_severity());
    add_line("Type", advisory.get_type());
    add_line("Status", advisory.get_status());
    add_line("Vendor", advisory.get_vendor());
    add_line("Issued", libdnf5::utils::string::format_epoch(advisory.get_buildtime()));
    add_lines("Description", libdnf5::utils::string::split(advisory.get_description(), "\n"));
    add_line("Message", advisory.get_message());
    add_line("Rights", advisory.get_rights());

    // References
    for (auto & reference : advisory.get_references()) {
        auto group_reference = add_line("Reference", "");
        add_line("Title", reference->get_title(), "bold", group_reference);
        add_line("Id", reference->get_id(), nullptr, group_reference);
        add_line("Type", reference->get_type_cstring(), nullptr, group_reference);
        add_line("Url", reference->get_url(), nullptr, group_reference);
    }

    // Collections
    for (auto & collection : advisory.get_collections()) {
        auto group_collection = add_line("Collection", "");

        // Modules
        auto modules = collection->get_modules();
        if (!modules.empty()) {
            auto module_iter = modules.begin();
            add_line("Modules", (*module_iter)->get_nsvca(), nullptr, group_collection);
            module_iter++;
            while (module_iter != modules.end()) {
                add_line("", (*module_iter)->get_nsvca(), nullptr, group_collection);
                module_iter++;
            }
        }

        // Packages
        auto packages = collection->get_packages();
        if (!packages.empty()) {
            auto package_iter = packages.begin();
            add_line("Packages", (*package_iter)->get_nevra(), nullptr, group_collection);
            package_iter++;
            while (package_iter != packages.end()) {
                add_line("", (*package_iter)->get_nevra(), nullptr, group_collection);
                package_iter++;
            }
        }
    }
}

void AdvisoryInfoJSON::Impl::add_advisory(IAdvisory & advisory) {
    json_object * json_advisory = json_object_new_object();
    json_object_object_add(json_advisory, "Name", json_object_new_string(advisory.get_name().c_str()));
    json_object_object_add(json_advisory, "Title", json_object_new_string(advisory.get_title().c_str()));
    json_object_object_add(json_advisory, "Severity", json_object_new_string(advisory.get_severity().c_str()));
    json_object_object_add(json_advisory, "Type", json_object_new_string(advisory.get_type().c_str()));
    json_object_object_add(json_advisory, "Status", json_object_new_string(advisory.get_status().c_str()));
    json_object_object_add(json_advisory, "Vendor", json_object_new_string(advisory.get_vendor().c_str()));
    json_object_object_add(
        json_advisory,
        "Issued",
        json_object_new_string(libdnf5::utils::string::format_epoch(advisory.get_buildtime()).c_str()));
    json_object_object_add(json_advisory, "Description", json_object_new_string(advisory.get_description().c_str()));
    json_object_object_add(json_advisory, "Message", json_object_new_string(advisory.get_message().c_str()));
    json_object_object_add(json_advisory, "Rights", json_object_new_string(advisory.get_rights().c_str()));

    // References
    json_object * json_references = json_object_new_array();
    for (auto & reference : advisory.get_references()) {
        json_object * json_reference = json_object_new_object();
        json_object_object_add(json_reference, "Title", json_object_new_string(reference->get_title().c_str()));
        json_object_object_add(json_reference, "Id", json_object_new_string(reference->get_id().c_str()));
        json_object_object_add(json_reference, "Type", json_object_new_string(reference->get_type_cstring()));
        json_object_object_add(json_reference, "Url", json_object_new_string(reference->get_url().c_str()));
        json_object_array_add(json_references, json_reference);
    }
    json_object_object_add(json_advisory, "references", json_references);

    // Collections
    json_object * json_collections = json_object_new_object();
    for (auto & collection : advisory.get_collections()) {
        // Modules
        auto modules = collection->get_modules();
        if (!modules.empty()) {
            json_object * json_modules = json_object_new_array();
            for (auto & module : modules) {
                json_object_array_add(json_modules, json_object_new_string(module->get_nsvca().c_str()));
            }
            json_object_object_add(json_collections, "modules", json_modules);
        }

        // Packages
        auto packages = collection->get_packages();
        if (!packages.empty()) {
            json_object * json_pkgs = json_object_new_array();
            for (auto & package : packages) {
                json_object_array_add(json_pkgs, json_object_new_string(package->get_nevra().c_str()));
            }
            json_object_object_add(json_collections, "packages", json_pkgs);
        }
        json_object_object_add(json_advisory, "collections", json_collections);
    }
    json_object_object_add(json_advisories, advisory.get_name().c_str(), json_advisory);
}


AdvisoryInfo::AdvisoryInfo() : p_impl{new AdvisoryInfo::Impl} {}
AdvisoryInfoJSON::AdvisoryInfoJSON() : p_impl{new AdvisoryInfoJSON::Impl} {}

AdvisoryInfo::~AdvisoryInfo() = default;
AdvisoryInfoJSON::~AdvisoryInfoJSON() = default;

void AdvisoryInfo::add_advisory(IAdvisory & advisory) {
    p_impl->add_advisory(advisory);
}

void AdvisoryInfoJSON::add_advisory(IAdvisory & advisory) {
    p_impl->add_advisory(advisory);
}

void AdvisoryInfo::print() {
    p_impl->print();
}

void AdvisoryInfoJSON::print() {
    p_impl->print();
}

}  // namespace libdnf5::cli::output
