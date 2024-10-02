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

#include "libdnf5-cli/output/modulelist.hpp"

#include "utils/string.hpp"

#include "libdnf5-cli/tty.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libsmartcols/libsmartcols.h>

#include <set>
#include <string>

namespace libdnf5::cli::output {

namespace {

// module list table columns
enum { COL_MODULE_NAME, COL_MODULE_STREAM, COL_MODULE_PROFILES, COL_MODULE_SUMMARY };

const std::string MODULELIST_TABLE_HINT = _("Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled");


struct libscols_table * create_modulelist_table() {
    struct libscols_table * table = scols_new_table();
    if (libdnf5::cli::tty::is_coloring_enabled()) {
        scols_table_enable_colors(table, 1);
    }
    struct libscols_column * cl = scols_table_new_column(table, "Name", 0.15, 0);
    scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
    scols_table_new_column(table, "Stream", 0.15, SCOLS_FL_TRUNC);
    scols_table_new_column(table, "Profiles", 0.3, SCOLS_FL_WRAP);
    scols_table_new_column(table, "Summary", 0.4, SCOLS_FL_WRAP);
    scols_table_enable_maxout(table, 1);
    return table;
}


void add_line_into_modulelist_table(
    struct libscols_table * table,
    const char * name,
    const char * stream,
    const char * profiles,
    const char * summary) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_MODULE_NAME, name);
    scols_line_set_data(ln, COL_MODULE_STREAM, stream);
    scols_line_set_data(ln, COL_MODULE_PROFILES, profiles);
    scols_line_set_data(ln, COL_MODULE_SUMMARY, summary);
}

}  // namespace

void print_modulelist_table(const std::vector<std::unique_ptr<IModuleItem>> & module_list) {
    struct libscols_table * table = create_modulelist_table();
    std::set<std::pair<std::string, std::string>> name_stream_pairs;
    for (auto & module_item : module_list) {
        const std::string & name = module_item->get_name();
        const std::string & stream = module_item->get_stream();
        if (!name_stream_pairs.contains(make_pair(name, stream))) {
            // Get stream string (append [d] and [e] or [x] if needed)
            const module::ModuleStatus & status = module_item->get_status();
            std::string stream_string = module_item->is_default() ? "[d]" : "";
            if (status == module::ModuleStatus::ENABLED) {
                stream_string.append("[e]");
            } else if (status == module::ModuleStatus::DISABLED) {
                stream_string.append("[x]");
            }
            stream_string = stream_string.empty() ? stream : stream + " " + stream_string;

            // Get profile strings (append [d] or [i] if needed)
            std::vector<std::string> profile_strings;
            for (const auto & profile : module_item->get_profiles()) {
                // TODO(pkratoch): Also show "[i]" for installed profiles
                profile_strings.push_back(profile->is_default() ? profile->get_name() + " [d]" : profile->get_name());
            }

            add_line_into_modulelist_table(
                table,
                name.c_str(),
                stream_string.c_str(),
                utils::string::join(profile_strings, ", ").c_str(),
                module_item->get_summary().c_str());
            name_stream_pairs.emplace(make_pair(name, stream));
        }
    }
    scols_sort_table(table, scols_table_get_column(table, COL_MODULE_NAME));
    scols_print_table(table);
    if (!module_list.empty()) {
        std::cout << std::endl;
    }
    scols_unref_table(table);
}


void print_modulelist_table_hint() {
    std::cout << MODULELIST_TABLE_HINT << std::endl;
}

}  // namespace libdnf5::cli::output
