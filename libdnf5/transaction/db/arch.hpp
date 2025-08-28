// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TRANSACTION_DB_ARCH_HPP
#define LIBDNF5_TRANSACTION_DB_ARCH_HPP

#include "utils/sqlite3/sqlite3.hpp"

#include <memory>

namespace libdnf5::transaction {

/// Create a query (statement) that inserts new records to the 'arch' table
std::unique_ptr<utils::SQLite3::Statement> arch_insert_if_not_exists_new_query(utils::SQLite3 & conn);

/// Use a query to insert a new record to the 'arch' table
int64_t arch_insert_if_not_exists(utils::SQLite3::Statement & query, const std::string & name);

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_DB_ARCH_HPP
