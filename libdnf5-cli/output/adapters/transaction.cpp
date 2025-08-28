// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/output/adapters/transaction.hpp"

namespace libdnf5::cli::output {

template class TransactionPackageAdapter<libdnf5::base::TransactionPackage>;
template class TransactionGroupAdapter<libdnf5::base::TransactionGroup>;
template class TransactionEnvironmentAdapter<libdnf5::base::TransactionEnvironment>;
template class TransactionModuleAdapter<libdnf5::base::TransactionModule>;
template class TransactionAdapter<libdnf5::base::Transaction>;

}  // namespace libdnf5::cli::output
