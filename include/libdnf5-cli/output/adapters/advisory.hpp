// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


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
