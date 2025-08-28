// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_PACKAGE_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_PACKAGE_HPP

#include "package_tmpl.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/rpm/package.hpp>

namespace libdnf5::cli::output {

extern template class LIBDNF_CLI_API PackageAdapter<libdnf5::rpm::Package>;

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_PACKAGE_HPP
