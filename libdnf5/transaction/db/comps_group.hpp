// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_TRANSACTION_DB_COMPS_GROUP_HPP
#define LIBDNF5_TRANSACTION_DB_COMPS_GROUP_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>


namespace libdnf5::transaction {


class CompsGroup;
class Transaction;

class CompsGroupDbUtils {
public:
    /// Return a vector of CompsGroup objects with comps groups in a transaction
    static std::vector<CompsGroup> get_transaction_comps_groups(libdnf5::utils::SQLite3 & conn, Transaction & trans);


    /// Use a query to insert a new record to the 'comps_group' table
    static int64_t comps_group_insert(libdnf5::utils::SQLite3::Statement & query, CompsGroup & grp);


    /// Insert CompsGroup objects associated with a transaction into the database
    static void insert_transaction_comps_groups(libdnf5::utils::SQLite3 & conn, Transaction & trans);
};


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_COMPS_GROUP_HPP
