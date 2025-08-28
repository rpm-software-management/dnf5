// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CLI_OUTPUT_PROVIDES_HPP
#define LIBDNF5_CLI_OUTPUT_PROVIDES_HPP

#include "interfaces/package.hpp"

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::output {

enum ProvidesMatchedBy : int { NO_MATCH = 0, PROVIDES = 1, FILENAME = 2, BINARY = 3 };

LIBDNF_CLI_API void print_provides_table(IPackage & package, const char * spec, int match);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PROVIDES_HPP
