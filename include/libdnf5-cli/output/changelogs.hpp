// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
