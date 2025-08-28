// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF_CLI_UTILS_UNITS
#define LIBDNF_CLI_UTILS_UNITS

#include "libdnf5-cli/defs.h"

#include <string>
#include <utility>


namespace libdnf5::cli::utils::units {


LIBDNF_CLI_API std::pair<float, const char *> to_size(int64_t num);

LIBDNF_CLI_API std::string format_size_aligned(int64_t num);


}  // namespace libdnf5::cli::utils::units


#endif  // LIBDNF_CLI_UTILS_UNITS
