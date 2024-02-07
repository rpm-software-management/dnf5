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


#ifndef LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP
#define LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP

#include <libdnf5/rpm/package_set.hpp>

namespace libdnf5::cli::output {

bool requires_filelists(const std::string & queryformat);

void print_pkg_set_with_format(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & queryformat);

void print_pkg_attr_uniq_sorted(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & getter_name);

void print_available_pkg_attrs(std::FILE * target);

libdnf5::rpm::ReldepList get_reldeplist_for_attr(
    const libdnf5::rpm::PackageSet & pkgs, const std::string & getter_name);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP
