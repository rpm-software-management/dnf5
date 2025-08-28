// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_TRANSACTION_DB_ITEM_HPP
#define LIBDNF5_TRANSACTION_DB_ITEM_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>


namespace libdnf5::transaction {


/// Create a query (statement) that inserts new records to the 'item' table
std::unique_ptr<libdnf5::utils::SQLite3::Statement> item_insert_new_query(libdnf5::utils::SQLite3 & conn);


/// Use a query to insert a new record to the 'item' table
int64_t item_insert(libdnf5::utils::SQLite3::Statement & query);


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_ITEM_HPP
