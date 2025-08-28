// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_REPOLIST_HPP
#define LIBDNF5_CLI_OUTPUT_REPOLIST_HPP

#include "interfaces/repo.hpp"

#include "libdnf5-cli/defs.h"

#include <memory>

namespace libdnf5::cli::output {

// repository list table columns
enum { COL_REPO_ID, COL_REPO_NAME, COL_REPO_STATUS };

LIBDNF_CLI_API void print_repolist_table(
    const std::vector<std::unique_ptr<IRepo>> & repos, bool with_status, size_t sort_column);
LIBDNF_CLI_API void print_repolist_json(const std::vector<std::unique_ptr<IRepo>> & repos);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_REPOLIST_HPP
