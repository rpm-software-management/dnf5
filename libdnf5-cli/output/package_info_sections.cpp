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

#include "libdnf5-cli/output/adapters/package.hpp"
#include "libdnf5-cli/output/packageinfo.hpp"

#include <libdnf5/rpm/nevra.hpp>

#include <algorithm>
#include <iostream>

namespace libdnf5::cli::output {

PackageInfoSections::PackageInfoSections() = default;

PackageInfoSections::~PackageInfoSections() = default;

void PackageInfoSections::print(const std::unique_ptr<PkgColorizer> & colorizer) {
    bool separator_needed = false;
    for (const auto & [heading, pkg_set, obsoletes] : p_impl->sections) {
        // sort the packages in section according to NEVRA
        std::vector<libdnf5::rpm::Package> packages;
        for (const auto & pkg : pkg_set) {
            packages.emplace_back(std::move(pkg));
        }
        std::sort(packages.begin(), packages.end(), libdnf5::rpm::cmp_nevra<libdnf5::rpm::Package>);


        if (!heading.empty()) {
            if (separator_needed) {
                std::cout << std::endl;
            }
            std::cout << heading << std::endl;
            separator_needed = false;
        }

        for (auto package : packages) {
            if (separator_needed) {
                std::cout << std::endl;
            }
            libdnf5::cli::output::PackageAdapter cli_pkg(package);
            auto obsoletes_it = obsoletes.find(package.get_id());
            if (obsoletes_it != obsoletes.end()) {
                libdnf5::cli::output::print_package_info(cli_pkg, colorizer, obsoletes_it->second);
            } else {
                libdnf5::cli::output::print_package_info(cli_pkg, colorizer);
            }
            separator_needed = true;
        }
    }
    return;
}

}  // namespace libdnf5::cli::output
