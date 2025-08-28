// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TRANSACTION_TRANSFORMERTRANSACTION_HPP
#define LIBDNF5_TRANSACTION_TRANSFORMERTRANSACTION_HPP


#include "transaction.hpp"

#include "libdnf5/transaction/db/trans_with.hpp"


namespace libdnf5::transaction {

/**
 * Class overrides default behavior with
 * inserting rows with explicitly set IDs
 */
class TransformerTransaction : public Transaction {
public:
    using Transaction::Transaction;
    void start() {
        dbInsert();
        save_transaction_runtime_packages(*this);
        saveItems();
    }
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSFORMERTRANSACTION_HPP
