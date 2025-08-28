// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_GROUPLIST_HPP
#define LIBDNF5_CLI_OUTPUT_GROUPLIST_HPP

#include "interfaces/comps.hpp"

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_grouplist_table(std::vector<std::unique_ptr<IGroup>> & group_list);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_GROUPLIST_HPP
