// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5-cli/output/provides.hpp"

#include "key_value_table.hpp"

#include <libdnf5/common/sack/match_string.hpp>
#include <libsmartcols/libsmartcols.h>

#include <iostream>

namespace libdnf5::cli::output {

namespace {

void add_line_into_provides_table(struct libscols_table * table, const char * key, const char * value) {
    struct libscols_line * ln = scols_table_new_line(table, nullptr);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);
}


struct libscols_table * create_provides_heading_table(IPackage & package) {
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 5, 0);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);

    add_line_into_provides_table(table, package.get_nevra().c_str(), package.get_summary().c_str());
    return table;
}


struct libscols_table * create_provides_table(IPackage & package, const char * spec, int match) {
    struct libscols_table * table = scols_new_table();
    // don't print provides if don't match for at least one
    if (!match) {
        return table;
    }
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 5, 0);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);

    add_line_into_provides_table(table, "Repo", package.get_repo_id().c_str());
    add_line_into_provides_table(table, "Matched From", "");
    switch (match) {
        case ProvidesMatchedBy::PROVIDES: {
            std::string spec_and_version =
                package.get_name() + " = " + package.get_version() + "-" + package.get_release();
            add_line_into_provides_table(table, "Provide", spec_and_version.c_str());
            break;
        }
        case ProvidesMatchedBy::FILENAME: {
            std::string pattern(spec);
            for (const auto & file : package.get_files()) {
                if (sack::match_string(file, sack::QueryCmp::GLOB, pattern)) {
                    add_line_into_provides_table(table, "Filename", file.c_str());
                }
            }
            break;
        }
        case ProvidesMatchedBy::BINARY: {
            std::string pattern(spec);
            std::vector<std::string> prefix{"/bin/", "/sbin/", "/usr/bin/", "/usr/sbin/"};
            for (const auto & file : package.get_files()) {
                for (const auto & p : prefix) {
                    if (sack::match_string(file, sack::QueryCmp::GLOB, p + pattern)) {
                        add_line_into_provides_table(table, "Filename", file.c_str());
                    }
                }
            }
            break;
        }
    }

    return table;
}

}  // namespace


void print_provides_table(IPackage & package, const char * spec, int match) {
    auto table_head = create_provides_heading_table(package);
    auto table = create_provides_table(package, spec, match);
    scols_print_table(table_head);
    scols_print_table(table);
    scols_unref_table(table_head);
    scols_unref_table(table);
    std::cout << std::endl;
}

}  // namespace libdnf5::cli::output
