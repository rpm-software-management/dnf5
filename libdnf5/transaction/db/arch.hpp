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
