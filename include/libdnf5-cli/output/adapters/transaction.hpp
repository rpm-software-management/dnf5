// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
