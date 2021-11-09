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


#ifndef LIBDNF_CLI_OUTPUT_ADVISORYLIST_HPP
#define LIBDNF_CLI_OUTPUT_ADVISORYLIST_HPP

#include "libdnf/advisory/advisory_package.hpp"

namespace libdnf::cli::output {

void print_advisorylist_table(
    std::vector<libdnf::advisory::AdvisoryPackage> & advisory_package_list,
    std::vector<libdnf::advisory::AdvisoryPackage> & advisory_package_list_installed);


}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_ADVISORYLIST_HPP
