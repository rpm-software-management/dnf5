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

#include "pkg_name.hpp"

namespace libdnf5::transaction {

static constexpr const char * SQL_NAME_INSERT_IF_NOT_EXISTS = R"**(
    INSERT INTO
        "pkg_name" (
            "name"
        )
    VALUES
        (?)
    ON CONFLICT DO NOTHING
    RETURNING "id"
)**";

std::unique_ptr<utils::SQLite3::Statement> pkg_name_insert_if_not_exists_new_query(utils::SQLite3 & conn) {
    return std::make_unique<utils::SQLite3::Statement>(conn, SQL_NAME_INSERT_IF_NOT_EXISTS);
}

int64_t pkg_name_insert_if_not_exists(utils::SQLite3::Statement & query, const std::string & name) {
    int64_t result = 0;

    query.bindv(name);
    if (query.step() == utils::SQLite3::Statement::StepResult::ROW) {
        result = query.get<int64_t>(0);
    }
    query.reset();
    return result;
}

}  // namespace libdnf5::transaction
