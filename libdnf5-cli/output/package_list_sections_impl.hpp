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


#ifndef LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_IMPL_HPP
#define LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_IMPL_HPP

#include "libdnf5-cli/output/package_list_sections.hpp"
#include "libdnf5-cli/tty.hpp"

#include <libsmartcols/libsmartcols.h>

#include <iostream>

namespace libdnf5::cli::output {

class PackageListSections::Impl {
public:
    Impl() {
        table = scols_new_table();
        scols_table_enable_noheadings(table, 1);
        if (libdnf5::cli::tty::is_interactive()) {
            scols_table_enable_colors(table, 1);
        }
    }

    ~Impl() { scols_unref_table(table); }

    void print();

    struct libscols_table * table = nullptr;
    // keeps track of the first and the last line of sections
    std::vector<std::tuple<std::string, struct libscols_line *, struct libscols_line *>> sections;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_IMPL_HPP
