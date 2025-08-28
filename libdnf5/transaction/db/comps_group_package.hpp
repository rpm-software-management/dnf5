// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_TRANSACTION_DB_COMPS_GROUP_PACKAGE_HPP
#define LIBDNF5_TRANSACTION_DB_COMPS_GROUP_PACKAGE_HPP


#include "utils/sqlite3/sqlite3.hpp"

#include "libdnf5/transaction/comps_group.hpp"


namespace libdnf5::transaction {


class CompsGroupPackageDbUtils {
public:
    /// Load GroupPackage objects from the database to the CompsGroup object
    static void comps_group_packages_select(libdnf5::utils::SQLite3 & conn, CompsGroup & group);


    /// Insert GroupPackage objects associated with a CompsGroup into the database
    static void comps_group_packages_insert(libdnf5::utils::SQLite3 & conn, CompsGroup & group);
};

}  // namespace libdnf5::transaction


#endif  // LIBDNF5_TRANSACTION_DB_COMPS_GROUP_PACKAGE_HPP
