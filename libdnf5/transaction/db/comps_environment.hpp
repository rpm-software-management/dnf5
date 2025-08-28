// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_TRANSACTION_DB_COMPS_ENVIRONMENT_HPP
#define LIBDNF5_TRANSACTION_DB_COMPS_ENVIRONMENT_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include <memory>


namespace libdnf5::transaction {


class CompsEnvironment;
class Transaction;


class CompsEnvironmentDbUtils {
public:
    /// Return a vector of CompsEnvironment objects with comps environments in a transaction
    static std::vector<CompsEnvironment> get_transaction_comps_environments(
        libdnf5::utils::SQLite3 & conn, Transaction & trans);

    /// Use a query to insert a new record to the 'comps_environment' table
    static int64_t comps_environment_insert(libdnf5::utils::SQLite3::Statement & query, CompsEnvironment & env);

    /// Insert CompsEnvironment objects associated with a transaction into the database
    static void insert_transaction_comps_environments(libdnf5::utils::SQLite3 & conn, Transaction & trans);
};


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_COMPS_ENVIRONMENT_HPP
