// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_TRANSACTIONINFO_HPP
#define LIBDNF5_CLI_OUTPUT_TRANSACTIONINFO_HPP

#include "libdnf5-cli/defs.h"

#include <libdnf5/transaction/transaction.hpp>

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_transaction_info(libdnf5::transaction::Transaction & transaction);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_TRANSACTIONINFO_HPP
