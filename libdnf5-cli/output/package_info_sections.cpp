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

#include "libdnf5-cli/output/package_info_sections.hpp"

#include "libdnf5-cli/tty.hpp"

#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <libsmartcols/libsmartcols.h>

#include <algorithm>
#include <iostream>
#include <string>

namespace libdnf5::cli::output {


enum { COL_KEY, COL_VALUE };

void PackageInfoSections::setup_cols() {
    scols_table_new_column(table, "key", 1, 0);
    scols_table_new_column(table, "value", 1, SCOLS_FL_WRAP);
    scols_table_set_column_separator(table, " : ");
}


struct libscols_line * PackageInfoSections::add_line(const std::string & key, const ::std::string & value) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_KEY, key.c_str());
    scols_line_set_data(ln, COL_VALUE, value.c_str());
    return ln;
}


bool PackageInfoSections::add_section(
    const std::string & heading,
    const libdnf5::rpm::PackageSet & pkg_set,
    const std::unique_ptr<PkgColorizer> & colorizer,
    const std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>> & obsoletes) {
    if (!pkg_set.empty()) {
        // sort the packages in section according to NEVRA
        std::vector<libdnf5::rpm::Package> packages;
        for (const auto & pkg : pkg_set) {
            packages.emplace_back(std::move(pkg));
        }
        std::sort(packages.begin(), packages.end(), libdnf5::rpm::cmp_nevra<libdnf5::rpm::Package>);

        auto tmp_heading = heading;
        for (const auto & pkg : packages) {
            auto obsoletes_it = obsoletes.find(pkg.get_id());
            if (obsoletes_it != obsoletes.end()) {
                add_package(pkg, tmp_heading, colorizer, obsoletes_it->second);
            } else {
                add_package(pkg, tmp_heading, colorizer, {});
            }
            tmp_heading = "";
            //TODO(amatej): This is a hacky workaround for a performance problem with
            //`scols_table_print_range()`. Once the table is large (eg. all pkgs in fedora repo)
            //calling `scols_table_print_range()` for each package is very slow.
            //However we cannot print the whole table at once because it isn't possible
            //to add an empty line into the table to separate the packages.
            //
            //To workaround this print each package right away after it is added and clear
            //the table. This changes the behavior, the printing is done by `add_section()` and the
            //final call to `print()` is a noop.
            //This should be reworked for 5.2.0.0 where we can break API.
            print();
            std::cout << '\n';
            sections.clear();
            scols_table_remove_lines(table);
        }
        return true;
    } else {
        return false;
    }
}


}  // namespace libdnf5::cli::output
