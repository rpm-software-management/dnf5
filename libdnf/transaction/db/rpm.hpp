/*
Copyright (C) 2017-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_TRANSACTION_DB_RPM_HPP
#define LIBDNF_TRANSACTION_DB_RPM_HPP


#include "libdnf/utils/sqlite3/sqlite3.hpp"

#include <memory>


namespace libdnf::transaction {


class Package;


/// Create a query that returns all rpm transaction items for a transaction
std::unique_ptr<libdnf::utils::SQLite3::Query> rpm_transaction_item_select_new_query(libdnf::utils::SQLite3 & conn, int64_t transaction_id);


/// Create a query (statement) that inserts new records to the 'rpm' table
std::unique_ptr<libdnf::utils::SQLite3::Statement> rpm_insert_new_query(libdnf::utils::SQLite3 & conn);


/// Use a query to insert a new record to the 'rpm' table
int64_t rpm_insert(libdnf::utils::SQLite3::Statement & query, const Package & rpm);


/// Create a query that returns primary keys from table 'rpm'
std::unique_ptr<libdnf::utils::SQLite3::Statement> rpm_select_pk_new_query(libdnf::utils::SQLite3 & conn);


/// Find a primary key of a recod in table 'rpm' that matches the Package.
/// Return an existing primary key or 0 if the record was not found.
int64_t rpm_select_pk(libdnf::utils::SQLite3::Statement & query, const Package & rpm);


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_DB_RPM_HPP
