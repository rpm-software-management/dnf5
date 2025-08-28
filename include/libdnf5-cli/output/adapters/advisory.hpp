// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_ADVISORY_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_ADVISORY_HPP

#include "advisory_tmpl.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/advisory/advisory.hpp>
#include <libdnf5/advisory/advisory_collection.hpp>
#include <libdnf5/advisory/advisory_module.hpp>
#include <libdnf5/advisory/advisory_package.hpp>
#include <libdnf5/advisory/advisory_reference.hpp>

namespace libdnf5::cli::output {

extern template class LIBDNF_CLI_API AdvisoryReferenceAdapter<libdnf5::advisory::AdvisoryReference>;
extern template class LIBDNF_CLI_API AdvisoryPackageAdapter<libdnf5::advisory::AdvisoryPackage>;
extern template class LIBDNF_CLI_API AdvisoryModuleAdapter<libdnf5::advisory::AdvisoryModule>;
extern template class LIBDNF_CLI_API AdvisoryCollectionAdapter<libdnf5::advisory::AdvisoryCollection>;
extern template class LIBDNF_CLI_API AdvisoryAdapter<libdnf5::advisory::Advisory>;

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_ADVISORY_HPP
