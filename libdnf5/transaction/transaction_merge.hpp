// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_MERGE_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_MERGE_HPP

#include "transaction_sr.hpp"


namespace libdnf5::transaction {

// Merge a vector of transactions replays.
// Order matters when merging transactions, we prefer the latest transaction actions (actions from TransactionReplays later in the vector).
// The na_to_installed_nevras is a unordered_map of currently installed nevras, in format name.arch: {nevra1, nevra2..} the vector is
// necessary because of installonly packages.
// The last argument is a vector of names of installonly packages.
//
// It returns the merged transaction and a vector of encountered problems.
std::tuple<TransactionReplay, std::vector<std::string>> merge_transactions(
    std::vector<TransactionReplay> transactions,
    std::unordered_map<std::string, std::vector<std::string>> & na_to_installed_nevras,
    std::vector<std::string> installonly_names = {});

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_MERGE_HPP
