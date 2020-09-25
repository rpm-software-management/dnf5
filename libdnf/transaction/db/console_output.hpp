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


#ifndef LIBDNF_TRANSACTION_DB_CONSOLE_OUTPUT_HPP
#define LIBDNF_TRANSACTION_DB_CONSOLE_OUTPUT_HPP


#include "libdnf/utils/sqlite3/sqlite3.hpp"

#include <memory>
#include <string>


namespace libdnf::transaction {


class Transaction;


/// Insert file_descriptor and line into the 'console_output' table.
int64_t console_output_insert_line(Transaction & trans, int file_descriptor, const std::string & line);


/// Load records from the 'console_output' table associated with a transaction.
/// Return vector of <file_descriptor, line> pairs.
std::vector<std::pair<int, std::string>> console_output_load(Transaction & trans);


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_DB_CONSOLE_OUTPUT_HPP
