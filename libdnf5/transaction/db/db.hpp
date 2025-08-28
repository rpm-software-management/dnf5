// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
