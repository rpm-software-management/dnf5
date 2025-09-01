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


#ifndef LIBDNF5_TRANSACTION_DB_REPO_HPP
#define LIBDNF5_TRANSACTION_DB_REPO_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>
#include <string>


namespace libdnf5::transaction {


/// Create a query (statement) that inserts new records to the 'repo' table
std::unique_ptr<libdnf5::utils::SQLite3::Statement> repo_insert_new_query(libdnf5::utils::SQLite3 & conn);


/// Use a query to insert a new record to the 'repo' table
int64_t repo_insert(libdnf5::utils::SQLite3::Statement & query, const std::string & repoid);


/// Create a query that returns primary keys from table 'repo'
std::unique_ptr<libdnf5::utils::SQLite3::Statement> repo_select_pk_new_query(libdnf5::utils::SQLite3 & conn);


/// Find a primary key of a record in table 'repo' that matches the Package.
/// Return an existing primary key or 0 if the record was not found.
int64_t repo_select_pk(libdnf5::utils::SQLite3::Statement & query, const std::string & repoid);


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_REPO_HPP
