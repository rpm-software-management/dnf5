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


#ifndef LIBDNF_TRANSACTION_DB_TRANS_WITH_HPP
#define LIBDNF_TRANSACTION_DB_TRANS_WITH_HPP


#include "libdnf/utils/sqlite3/sqlite3.hpp"

#include <set>
#include <string>


namespace libdnf::transaction {


class Transaction;


/// Load records from table 'rpm' associated with a transaction via 'trans_with' table.
/// Return set of package NEVRAs.
std::set<std::string> load_transaction_runtime_packages(Transaction & trans);


/// Insert transaction runtime packages to 'rpm' and 'trans_with' tables.
void save_transaction_runtime_packages(Transaction & trans);


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_DB_TRANS_WITH_HPP
