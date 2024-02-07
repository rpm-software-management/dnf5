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

#include "package_list_sections_impl.hpp"
#include "utils/string.hpp"

#include "libdnf5-cli/output/adapters/package.hpp"
#include "libdnf5-cli/utils/units.hpp"

#include <libdnf5/rpm/nevra.hpp>

#include <algorithm>

namespace libdnf5::cli::output {

namespace {

enum { COL_KEY, COL_VALUE };

struct libscols_line * add_line(struct libscols_table * table, const std::string & key, const ::std::string & value) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_KEY, key.c_str());
    scols_line_set_data(ln, COL_VALUE, value.c_str());
    return ln;
}

}  // namespace


PackageInfoSections::PackageInfoSections() = default;

PackageInfoSections::~PackageInfoSections() = default;


bool PackageInfoSections::add_package(
    IPackage & pkg,
    const std::string & heading,
    const std::unique_ptr<PkgColorizer> & colorizer,
    const std::vector<libdnf5::rpm::Package> & obsoletes) {
    auto * table = p_impl->table;
    struct libscols_line * first_line = add_line(table, "Name", pkg.get_name());
    if (colorizer) {
        scols_line_set_color(first_line, colorizer->get_pkg_color(pkg).c_str());
    }

    add_line(table, "Epoch", pkg.get_epoch());
    add_line(table, "Version", pkg.get_version());
    add_line(table, "Release", pkg.get_release());
    add_line(table, "Architecture", pkg.get_arch());

    if (!obsoletes.empty()) {
        auto iterator = obsoletes.begin();
        add_line(table, "Obsoletes", iterator->get_full_nevra());
        ++iterator;
        for (; iterator != obsoletes.end(); ++iterator) {
            add_line(table, "", iterator->get_full_nevra());
        }
    }

    if (!pkg.is_installed()) {
        add_line(
            table, "Download size", utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_download_size())));
    }
    add_line(table, "Installed size", utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_install_size())));
    if (pkg.get_arch() != "src") {
        add_line(table, "Source", pkg.get_sourcerpm());
    }
    if (pkg.is_installed()) {
        add_line(table, "From repository", pkg.get_from_repo_id());
    } else {
        add_line(table, "Repository", pkg.get_repo_id());
    }
    add_line(table, "Summary", pkg.get_summary());
    add_line(table, "URL", pkg.get_url());
    add_line(table, "License", pkg.get_license());

    auto lines = libdnf5::utils::string::split(pkg.get_description(), "\n");
    auto iterator = lines.begin();
    add_line(table, "Description", *iterator);
    ++iterator;
    for (; iterator != lines.end(); ++iterator) {
        add_line(table, "", *iterator);
    }
    struct libscols_line * last_line = add_line(table, "Vendor", pkg.get_vendor());

    // for info output keep each package as a separate section, which means
    // an empty line is printed between packages resulting in better readability
    p_impl->sections.emplace_back(heading, first_line, last_line);
    return true;
}


void PackageInfoSections::setup_cols() {
    scols_table_new_column(p_impl->table, "key", 1, 0);
    scols_table_new_column(p_impl->table, "value", 1, SCOLS_FL_WRAP);
    scols_table_set_column_separator(p_impl->table, " : ");
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
            PackageAdapter cli_pkg(pkg);
            if (obsoletes_it != obsoletes.end()) {
                add_package(cli_pkg, tmp_heading, colorizer, obsoletes_it->second);
            } else {
                add_package(cli_pkg, tmp_heading, colorizer, {});
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
            p_impl->sections.clear();
            scols_table_remove_lines(p_impl->table);
        }
        return true;
    } else {
        return false;
    }
}

}  // namespace libdnf5::cli::output
