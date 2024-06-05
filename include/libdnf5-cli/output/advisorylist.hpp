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


#ifndef LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP
#define LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP

#include "interfaces/advisory.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/advisory/advisory_package.hpp>
#include <libdnf5/advisory/advisory_reference.hpp>

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_advisorylist_table(
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_not_installed,
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_installed);

LIBDNF_CLI_API void print_advisorylist_json(
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_not_installed,
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_installed);

LIBDNF_CLI_API void print_advisorylist_references_table(
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_installed,
    std::string reference_type);

LIBDNF_CLI_API void print_advisorylist_references_json(
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_installed,
    std::string reference_type);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP
