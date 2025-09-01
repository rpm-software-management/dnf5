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


#ifndef LIBDNF5_TRANSACTION_DB_DB_HPP
#define LIBDNF5_TRANSACTION_DB_DB_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>


namespace libdnf5 {
class Base;
}


namespace libdnf5::transaction {


/// Create a connection to transaction database in the 'persistdir' directory.
/// The file is named 'transaction_history.sqlite'.
std::unique_ptr<libdnf5::utils::SQLite3> transaction_db_connect(libdnf5::Base & base);


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_DB_HPP
