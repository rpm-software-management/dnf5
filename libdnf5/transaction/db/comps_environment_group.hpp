// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_TRANSACTION_DB_COMPS_ENVIRONMENT_GROUP_HPP
#define LIBDNF5_TRANSACTION_DB_COMPS_ENVIRONMENT_GROUP_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include "libdnf5/transaction/comps_environment.hpp"


namespace libdnf5::transaction {

class CompsEnvironmentGroupDbUtils {
public:
    /// Load EnvironmentGroup objects from the database to the CompsEnvironment object
    static void comps_environment_groups_select(libdnf5::utils::SQLite3 & conn, CompsEnvironment & env);

    /// Insert EnvironmentGroup objects associated with a CompsEnvironment into the database
    static void comps_environment_groups_insert(libdnf5::utils::SQLite3 & conn, CompsEnvironment & env);
};


}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_COMPS_ENVIRONMENT_GROUP_HPP
