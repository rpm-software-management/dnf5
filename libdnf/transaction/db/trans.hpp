/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_TRANSACTION_DB_TRANS_HPP
#define LIBDNF_TRANSACTION_DB_TRANS_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include "libdnf/base/base_weak.hpp"

#include <memory>


namespace libdnf::transaction {


class Transaction;


/// Selects all transaction IDs, sorting in ascending order.
std::vector<int64_t> select_transaction_ids(const BaseWeakPtr & base);


/// Selects transactions using a list of ids, in case `ids` is empty, selects all transactions.
std::vector<Transaction> select_transactions_by_ids(const BaseWeakPtr & base, const std::vector<int64_t> & ids);


/// Selects transactions with ids within the [start, end] range (inclusive).
std::vector<Transaction> select_transactions_by_range(const BaseWeakPtr & base, int64_t start, int64_t end);


/// Create a query for inserting records to the 'trans' table
std::unique_ptr<libdnf::utils::SQLite3::Statement> trans_insert_new_query(libdnf::utils::SQLite3 & conn);


/// Use a query to insert a new record to the 'trans' table
void trans_insert(libdnf::utils::SQLite3::Statement & query, Transaction & trans);


/// Create a query that updates a record in 'trans' table
std::unique_ptr<libdnf::utils::SQLite3::Statement> trans_update_new_query(libdnf::utils::SQLite3 & conn);


/// Use a query to update a record in the 'trans' table
void trans_update(libdnf::utils::SQLite3::Statement & query, Transaction & trans);


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_DB_TRANS_HPP
