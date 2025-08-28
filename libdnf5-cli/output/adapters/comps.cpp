// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/output/adapters/comps.hpp"

namespace libdnf5::cli::output {

template class GroupPackageAdapter<libdnf5::comps::Package>;
template class GroupAdapter<libdnf5::comps::Group>;
template class EnvironmentAdapter<libdnf5::comps::Environment>;

}  // namespace libdnf5::cli::output
