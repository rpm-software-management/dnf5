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


#ifndef LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
#define LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP

#include "interfaces/package.hpp"
#include "package_list_sections.hpp"

namespace libdnf5::cli::output {

class PackageInfoSections : public PackageListSections {
public:
    PackageInfoSections();
    ~PackageInfoSections();

    bool add_package(
        IPackage & pkg,
        const std::string & heading = "",
        const std::unique_ptr<PkgColorizer> & colorizer = nullptr,
        const std::vector<libdnf5::rpm::Package> & obsoletes = {});

    bool add_section(
        const std::string & heading,
        const libdnf5::rpm::PackageSet & pkg_set,
        const std::unique_ptr<PkgColorizer> & colorizer = nullptr,
        const std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>> & obsoletes = {}) override;

    void setup_cols() override;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
