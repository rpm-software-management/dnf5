// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/output/adapters/advisory.hpp"

namespace libdnf5::cli::output {

template class AdvisoryReferenceAdapter<libdnf5::advisory::AdvisoryReference>;
template class AdvisoryPackageAdapter<libdnf5::advisory::AdvisoryPackage>;
template class AdvisoryModuleAdapter<libdnf5::advisory::AdvisoryModule>;
template class AdvisoryCollectionAdapter<libdnf5::advisory::AdvisoryCollection>;
template class AdvisoryAdapter<libdnf5::advisory::Advisory>;

}  // namespace libdnf5::cli::output
