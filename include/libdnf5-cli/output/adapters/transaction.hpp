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


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_TRANSACTION_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_TRANSACTION_HPP

#include "transaction_tmpl.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/base/transaction.hpp>
#include <libdnf5/base/transaction_environment.hpp>
#include <libdnf5/base/transaction_group.hpp>
#include <libdnf5/base/transaction_module.hpp>
#include <libdnf5/base/transaction_package.hpp>

namespace libdnf5::cli::output {

extern template class LIBDNF_CLI_API TransactionPackageAdapter<libdnf5::base::TransactionPackage>;
extern template class LIBDNF_CLI_API TransactionGroupAdapter<libdnf5::base::TransactionGroup>;
extern template class LIBDNF_CLI_API TransactionEnvironmentAdapter<libdnf5::base::TransactionEnvironment>;
extern template class LIBDNF_CLI_API TransactionModuleAdapter<libdnf5::base::TransactionModule>;
extern template class LIBDNF_CLI_API TransactionAdapter<libdnf5::base::Transaction>;

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_TRANSACTION_HPP
