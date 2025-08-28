// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF_CLI_OUTPUT_MODULELIST_HPP
#define LIBDNF_CLI_OUTPUT_MODULELIST_HPP

#include "interfaces/module.hpp"

#include "libdnf5-cli/defs.h"

#include <memory>
#include <vector>

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_modulelist_table(const std::vector<std::unique_ptr<IModuleItem>> & module_list);

LIBDNF_CLI_API void print_modulelist_table_hint();

}  // namespace libdnf5::cli::output

#endif  // LIBDNF_CLI_OUTPUT_MODULELIST_HPP
