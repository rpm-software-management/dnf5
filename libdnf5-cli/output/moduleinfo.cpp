// Copyright Contributors to the DNF5 project.
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


#include "libdnf5-cli/output/moduleinfo.hpp"

#include "key_value_table.hpp"
#include "utils/string.hpp"

#include <libdnf5/module/module_item.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <iostream>
#include <map>

namespace libdnf5::cli::output {

namespace {

const std::string MODULEINFO_TABLE_HINT = _("Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive");

class ModuleInfo : public KeyValueTable {
public:
    void add_module_item(IModuleItem & module_item);

private:
    void add_multiline_value(const char * key, const std::vector<std::string> & multiline_value);
};

void ModuleInfo::add_multiline_value(const char * key, const std::vector<std::string> & values) {
    if (values.empty()) {
        add_line(key, "");
        return;
    }
    auto it = values.begin();
    // put the first item at the same line as description
    add_line(key, it->c_str());
    it++;

    // put the remaining items on separate lines
    for (; it != values.end(); it++) {
        add_line("", it->c_str());
    }
}

void ModuleInfo::add_module_item(IModuleItem & module_item) {
    std::vector<std::string> profile_names;
    for (const auto & profile : module_item.get_profiles()) {
        profile_names.push_back(profile->get_name() + (profile->is_default() ? " [d]" : ""));
    }

    std::vector<std::string> dependency_strings;
    for (const auto & dependency : module_item.get_module_dependencies()) {
        dependency_strings.push_back(dependency->to_string());
    }

    // Get stream string (append [d] and [e] or [x] if needed)
    const auto status = module_item.get_status();
    std::string stream_string = module_item.is_default() ? "[d]" : "";
    if (status == module::ModuleStatus::ENABLED) {
        stream_string.append("[e]");
    } else if (status == module::ModuleStatus::DISABLED) {
        stream_string.append("[x]");
    }
    if (module_item.is_active()) {
        stream_string.append("[a]");
    }
    stream_string = stream_string.empty() ? module_item.get_stream() : module_item.get_stream() + " " + stream_string;

    // Trim summary and description
    auto summary = module_item.get_summary();
    libdnf5::utils::string::trim(summary);
    auto description = module_item.get_description();
    libdnf5::utils::string::trim(description);

    add_line("Name", module_item.get_name());
    add_line("Stream", stream_string);
    add_line("Version", module_item.get_version_str());
    add_line("Context", module_item.get_context());
    add_line("Architecture", module_item.get_arch());
    add_line("Profiles", utils::string::join(profile_names, ", "));
    add_line("Default profiles", utils::string::join(module_item.get_default_profiles(), ", "));
    add_line("Repo", module_item.get_repo_id());
    add_multiline_value("Summary", libdnf5::utils::string::split(summary, "\n"));
    add_multiline_value("Description", libdnf5::utils::string::split(description, "\n"));
    add_multiline_value("Requires", dependency_strings);
    add_multiline_value("Artifacts", module_item.get_artifacts());
}

}  // namespace


void print_module_item(IModuleItem & module_item) {
    libdnf5::cli::output::ModuleInfo module_info;
    module_info.add_module_item(module_item);
    module_info.print();
    std::cout << std::endl;
}


void print_moduleinfo_table(std::vector<std::unique_ptr<IModuleItem>> & module_list) {
    std::map<std::string, IModuleItem *> module_map;
    for (const auto & module_item : module_list) {
        module_map.emplace(module_item->get_full_identifier() + ":" + module_item->get_repo_id(), module_item.get());
    }
    for (auto & module_item : module_map) {
        print_module_item(*module_item.second);
    }
}


void print_moduleinfo_table_hint() {
    std::cout << MODULEINFO_TABLE_HINT << std::endl;
}

}  // namespace libdnf5::cli::output
