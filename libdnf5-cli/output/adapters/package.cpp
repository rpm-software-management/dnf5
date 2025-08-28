// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/output/adapters/package.hpp"

namespace libdnf5::cli::output {

template class PackageAdapter<libdnf5::rpm::Package>;

}  // namespace libdnf5::cli::output
