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


#ifndef LIBDNF_TRANSACTION_DB_COMPS_ENVIRONMENT_GROUP_HPP
#define LIBDNF_TRANSACTION_DB_COMPS_ENVIRONMENT_GROUP_HPP


#include "libdnf/transaction/comps_environment.hpp"


namespace libdnf::transaction {


/// Load EnvironmentGroup objects from the database to the CompsEnvironment object
void comps_environment_groups_select(CompsEnvironment & env);


/// Insert EnvironmentGroup objects associated with a CompsEnvironment into the database
void comps_environment_groups_insert(CompsEnvironment & env);


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_DB_COMPS_ENVIRONMENT_GROUP_HPP
