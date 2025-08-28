// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF_CLI_OUTPUT_PACKAGEINFO_HPP
#define LIBDNF_CLI_OUTPUT_PACKAGEINFO_HPP

#include "interfaces/package.hpp"
#include "pkg_colorizer.hpp"

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_package_info(
    IPackage & pkg,
    const std::unique_ptr<PkgColorizer> & colorizer = nullptr,
    const std::vector<libdnf5::rpm::Package> & obsoletes = {});

}  // namespace libdnf5::cli::output

#endif  // LIBDNF_CLI_OUTPUT_PACKAGEINFO_HPP
