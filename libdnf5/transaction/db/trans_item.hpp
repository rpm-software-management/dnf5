// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_TRANSACTION_DB_TRANS_ITEM_HPP
#define LIBDNF5_TRANSACTION_DB_TRANS_ITEM_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>


namespace libdnf5::transaction {


class TransactionItem;

class TransItemDbUtils {
public:
    /// Copy 'trans_item' fields from a query to TransactionItem or an object that inherits from it
    static void transaction_item_select(libdnf5::utils::SQLite3::Query & query, TransactionItem & ti);


    /// Create a query (statement) that inserts new records to the 'trans_item' table
    static std::unique_ptr<libdnf5::utils::SQLite3::Statement> trans_item_insert_new_query(
        libdnf5::utils::SQLite3 & conn);


    /// Use a query to insert a new record to the 'trans_item' table
    static int64_t transaction_item_insert(libdnf5::utils::SQLite3::Statement & query, TransactionItem & ti);
};


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_TRANS_ITEM_HPP
