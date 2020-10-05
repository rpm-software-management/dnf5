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


#include "item.hpp"


namespace libdnf::transaction {


static const char * SQL_ITEM_INSERT = R"**(
    INSERT INTO
        item (
            item_type
        )
    VALUES
        (?)
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> item_insert_new_query(libdnf::utils::SQLite3 & conn, TransactionItemType type) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_ITEM_INSERT);
    query->bindv(static_cast<int>(type));
    return query;
}


int64_t item_insert(libdnf::utils::SQLite3::Statement & query) {
    if (query.step() != libdnf::utils::SQLite3::Statement::StepResult::DONE) {
        // TODO(dmach): replace with a better exception class
        throw std::runtime_error("");
    }
    return query.last_insert_rowid();
}


}  // namespace libdnf::transaction
