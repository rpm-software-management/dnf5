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

#include "libdnf5-cli/output/package_list_sections.hpp"

#include "package_list_sections_impl.hpp"

#include "libdnf5-cli/output/adapters/package.hpp"

#include <libdnf5/rpm/nevra.hpp>

#include <algorithm>

namespace libdnf5::cli::output {

void PackageListSections::Impl::print() {
    // smartcols does not support spanning the text among multiple cells to create
    // heading lines. To create sections, print the headings separately and than print
    // appropriate range of the table.
    bool separator_needed = false;
    for (const auto & [heading, first, last] : sections) {
        if (separator_needed) {
            std::cout << std::endl;
        }
        if (!heading.empty()) {
            std::cout << heading << std::endl;
        }
        scols_table_print_range(table, first, last);
        separator_needed = true;
    }
    if (!sections.empty()) {
        std::cout << std::endl;
    }
}


PackageListSections::PackageListSections() : p_impl{new Impl} {}


PackageListSections::~PackageListSections() = default;


void PackageListSections::print() {
    p_impl->print();
}


void PackageListSections::setup_cols() {
    scols_table_new_column(p_impl->table, "Name", 1, 0);
    scols_table_new_column(p_impl->table, "Version", 1, 0);
    scols_table_new_column(p_impl->table, "Repository", 1, SCOLS_FL_TRUNC);
}


bool PackageListSections::add_section(
    const std::string & heading,
    const libdnf5::rpm::PackageSet & pkg_set,
    const std::unique_ptr<PkgColorizer> & colorizer,
    const std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>> & obsoletes) {
    enum { COL_NA, COL_EVR, COL_REPO };
    if (!pkg_set.empty()) {
        // sort the packages in section according to NEVRA
        std::vector<libdnf5::rpm::Package> packages;
        for (const auto & pkg : pkg_set) {
            packages.emplace_back(std::move(pkg));
        }
        std::sort(packages.begin(), packages.end(), libdnf5::rpm::cmp_nevra<libdnf5::rpm::Package>);

        struct libscols_line * first_line = nullptr;
        struct libscols_line * last_line = nullptr;
        for (const auto & pkg : packages) {
            struct libscols_line * ln = scols_table_new_line(p_impl->table, NULL);
            if (first_line == nullptr) {
                first_line = ln;
            }
            last_line = ln;
            if (colorizer) {
                scols_line_set_color(ln, colorizer->get_pkg_color(PackageAdapter(pkg)).c_str());
            }
            scols_line_set_data(ln, COL_NA, pkg.get_na().c_str());
            scols_line_set_data(ln, COL_EVR, pkg.get_evr().c_str());
            if (pkg.is_installed()) {
                scols_line_set_data(ln, COL_REPO, pkg.get_from_repo_id().c_str());
            } else {
                scols_line_set_data(ln, COL_REPO, pkg.get_repo_id().c_str());
            }

            auto obsoletes_it = obsoletes.find(pkg.get_id());
            if (obsoletes_it != obsoletes.end() && !obsoletes_it->second.empty()) {
                for (const auto & pkg_ob : obsoletes_it->second) {
                    struct libscols_line * ln = scols_table_new_line(p_impl->table, NULL);
                    last_line = ln;
                    scols_line_set_data(ln, COL_NA, ("    " + pkg_ob.get_na()).c_str());
                    scols_line_set_data(ln, COL_EVR, pkg_ob.get_evr().c_str());
                    scols_line_set_data(ln, COL_REPO, pkg_ob.get_from_repo_id().c_str());
                }
            }
        }
        p_impl->sections.emplace_back(heading, first_line, last_line);
        return true;
    } else {
        return false;
    }
}

}  // namespace libdnf5::cli::output
