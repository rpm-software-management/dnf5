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
