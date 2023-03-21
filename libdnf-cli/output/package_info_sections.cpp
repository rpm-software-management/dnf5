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

#include "libdnf-cli/output/package_info_sections.hpp"

#include "utils/string.hpp"

#include "libdnf-cli/tty.hpp"
#include "libdnf-cli/utils/units.hpp"

#include "libdnf/rpm/nevra.hpp"
#include "libdnf/rpm/package_set.hpp"

#include <libsmartcols/libsmartcols.h>

#include <algorithm>
#include <iostream>
#include <string>

namespace libdnf::cli::output {


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
    const libdnf::rpm::PackageSet & pkg_set,
    const std::unique_ptr<PkgColorizer> & colorizer,
    const std::map<libdnf::rpm::PackageId, std::vector<libdnf::rpm::Package>> & obsoletes) {
    if (!pkg_set.empty()) {
        // sort the packages in section according to NEVRA
        std::vector<libdnf::rpm::Package> packages;
        for (const auto & pkg : pkg_set) {
            packages.emplace_back(std::move(pkg));
        }
        std::sort(packages.begin(), packages.end(), libdnf::rpm::cmp_nevra<libdnf::rpm::Package>);

        struct libscols_line * first_line = nullptr;
        struct libscols_line * last_line = nullptr;
        auto tmp_heading = heading;
        for (const auto & pkg : packages) {
            auto ln = add_line("Name", pkg.get_name());
            if (colorizer) {
                scols_line_set_color(ln, colorizer->get_pkg_color(pkg).c_str());
            }
            first_line = ln;

            add_line("Epoch", pkg.get_epoch());
            add_line("Version", pkg.get_version());
            add_line("Release", pkg.get_release());
            add_line("Architecture", pkg.get_arch());
            auto obsoletes_it = obsoletes.find(pkg.get_id());
            if (obsoletes_it != obsoletes.end() && !obsoletes_it->second.empty()) {
                auto iterator = obsoletes_it->second.begin();
                add_line("Obsoletes", iterator->get_full_nevra());
                ++iterator;
                for (; iterator != obsoletes_it->second.end(); ++iterator) {
                    ln = add_line("", iterator->get_full_nevra());
                }
            }
            if (!pkg.is_installed()) {
                add_line(
                    "Package size", utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_package_size())));
            }
            add_line("Installed size", utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_install_size())));
            if (pkg.get_arch() != "src") {
                add_line("Source", pkg.get_sourcerpm());
            }
            if (pkg.is_installed()) {
                add_line("From repository", pkg.get_from_repo_id());
            } else {
                add_line("Repository", pkg.get_repo_id());
            }
            add_line("Summary", pkg.get_summary());
            add_line("URL", pkg.get_url());
            add_line("License", pkg.get_license());

            auto lines = libdnf::utils::string::split(pkg.get_description(), "\n");
            auto iterator = lines.begin();
            ln = add_line("Description", *iterator);
            ++iterator;
            for (; iterator != lines.end(); ++iterator) {
                ln = add_line("", *iterator);
            }
            last_line = ln;
            // for info output keep each package as a separate section, which means
            // an empty line is printed between packages resulting in better readability
            sections.emplace_back(tmp_heading, first_line, last_line);
            tmp_heading = "";
        }
        return true;
    } else {
        return false;
    }
}


}  // namespace libdnf::cli::output
