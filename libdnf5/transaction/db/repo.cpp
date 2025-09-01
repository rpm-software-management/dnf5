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


#include "repo.hpp"


namespace libdnf5::transaction {


static constexpr const char * SQL_REPO_INSERT = R"**(
    INSERT INTO
        "repo" (
            "repoid"
        )
    VALUES
        (?)
)**";


std::unique_ptr<libdnf5::utils::SQLite3::Statement> repo_insert_new_query(libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_REPO_INSERT);
    return query;
}


int64_t repo_insert(libdnf5::utils::SQLite3::Statement & query, const std::string & repoid) {
    query.bindv(repoid);
    query.step();
    int64_t result = query.last_insert_rowid();
    query.reset();
    return result;
}


static constexpr const char * SQL_REPO_SELECT_PK = R"**(
    SELECT
        "id"
    FROM
        "repo"
    WHERE
        "repoid" = ?
)**";


std::unique_ptr<libdnf5::utils::SQLite3::Statement> repo_select_pk_new_query(libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_REPO_SELECT_PK);
    return query;
}


int64_t repo_select_pk(libdnf5::utils::SQLite3::Statement & query, const std::string & repoid) {
    query.bindv(repoid);

    int64_t result = 0;
    if (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        result = query.get<int64_t>(0);
    }
    query.reset();
    return result;
}


}  // namespace libdnf5::transaction
