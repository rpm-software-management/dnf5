// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/output/adapters/repo.hpp"

#include <libdnf5/repo/repo.hpp>

namespace libdnf5::cli::output {

template class RepoAdapter<libdnf5::repo::RepoWeakPtr>;

}  // namespace libdnf5::cli::output
