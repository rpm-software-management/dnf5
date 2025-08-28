// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_REPO_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_REPO_HPP

#include "repo_tmpl.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/repo/repo_weak.hpp>

namespace libdnf5::cli::output {

extern template class LIBDNF_CLI_API RepoAdapter<libdnf5::repo::RepoWeakPtr>;

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_REPO_HPP
