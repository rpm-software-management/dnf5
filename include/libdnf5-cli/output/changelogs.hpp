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


#ifndef LIBDNF5_CLI_OUTPUT_CHANGELOGS_HPP
#define LIBDNF5_CLI_OUTPUT_CHANGELOGS_HPP

#include "libdnf5-cli/defs.h"

#include <libdnf5/rpm/package_query.hpp>

#include <string>
#include <variant>


namespace libdnf5::cli::output {

enum class ChangelogFilterType { NONE, UPGRADES, COUNT, SINCE };

LIBDNF_CLI_API void print_changelogs(
    libdnf5::rpm::PackageQuery & query,
    std::pair<ChangelogFilterType, std::variant<libdnf5::rpm::PackageQuery, int64_t, int32_t>> filter);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_CHANGELOGS_HPP
