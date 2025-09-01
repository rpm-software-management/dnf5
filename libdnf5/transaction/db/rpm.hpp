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


#ifndef LIBDNF5_TRANSACTION_DB_RPM_HPP
#define LIBDNF5_TRANSACTION_DB_RPM_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>
#include <vector>


namespace libdnf5::transaction {


class Package;
class Transaction;

class RpmDbUtils {
public:
    /// Use a query to select a record from the 'rpm' table and populate a TransactionItem
    static int64_t rpm_transaction_item_select(libdnf5::utils::SQLite3::Query & query, Package & pkg);


    /// Use a query to insert a new record to the 'rpm' table
    static int64_t rpm_insert(libdnf5::utils::SQLite3::Statement & query, const Package & rpm);


    /// Find a primary key of a record in table 'rpm' that matches the Package.
    /// Return an existing primary key or 0 if the record was not found.
    static int64_t rpm_select_pk(libdnf5::utils::SQLite3::Statement & query, const Package & rpm);


    /// Use a query to select a record from 'rpm' table and populate a Package
    static bool rpm_select(libdnf5::utils::SQLite3::Query & query, int64_t rpm_id, Package & rpm);


    /// Return a vector of Package objects with packages in a transaction
    static std::vector<Package> get_transaction_packages(libdnf5::utils::SQLite3 & conn, Transaction & trans);


    /// Insert Package objects associated with a transaction into the database
    static void insert_transaction_packages(libdnf5::utils::SQLite3 & conn, Transaction & trans);
};


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_RPM_HPP
