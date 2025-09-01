// Copyright Contributors to the DNF5 project.
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


#include "libdnf5-cli/output/adapters/advisory.hpp"

namespace libdnf5::cli::output {

template class AdvisoryReferenceAdapter<libdnf5::advisory::AdvisoryReference>;
template class AdvisoryPackageAdapter<libdnf5::advisory::AdvisoryPackage>;
template class AdvisoryModuleAdapter<libdnf5::advisory::AdvisoryModule>;
template class AdvisoryCollectionAdapter<libdnf5::advisory::AdvisoryCollection>;
template class AdvisoryAdapter<libdnf5::advisory::Advisory>;

}  // namespace libdnf5::cli::output
