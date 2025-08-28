// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_COMPS_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_COMPS_HPP

#include "comps_tmpl.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/comps/environment/environment.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/package.hpp>

namespace libdnf5::cli::output {

extern template class LIBDNF_CLI_API GroupPackageAdapter<libdnf5::comps::Package>;
extern template class LIBDNF_CLI_API GroupAdapter<libdnf5::comps::Group>;
extern template class LIBDNF_CLI_API EnvironmentAdapter<libdnf5::comps::Environment>;

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_COMPS_HPP
