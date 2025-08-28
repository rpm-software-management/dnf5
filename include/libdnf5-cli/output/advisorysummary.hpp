// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADVISORYSUMMARY_HPP
#define LIBDNF5_CLI_OUTPUT_ADVISORYSUMMARY_HPP

#include "libdnf5-cli/defs.h"

#include <libdnf5/advisory/advisory_query.hpp>

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_advisorysummary_table(
    const libdnf5::advisory::AdvisoryQuery & advisories, const std::string & mode);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADVISORYSUMMARY_HPP
