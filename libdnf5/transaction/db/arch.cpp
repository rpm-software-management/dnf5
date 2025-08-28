// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "arch.hpp"

namespace libdnf5::transaction {

static constexpr const char * SQL_NAME_INSERT_IF_NOT_EXISTS = R"**(
    INSERT INTO
        "arch" (
            "name"
        )
    VALUES
        (?)
    ON CONFLICT DO NOTHING
    RETURNING "id"
)**";

std::unique_ptr<utils::SQLite3::Statement> arch_insert_if_not_exists_new_query(utils::SQLite3 & conn) {
    return std::make_unique<utils::SQLite3::Statement>(conn, SQL_NAME_INSERT_IF_NOT_EXISTS);
}

int64_t arch_insert_if_not_exists(utils::SQLite3::Statement & query, const std::string & name) {
    int64_t result = 0;

    query.bindv(name);
    if (query.step() == utils::SQLite3::Statement::StepResult::ROW) {
        result = query.get<int64_t>(0);
    }
    query.reset();
    return result;
}

}  // namespace libdnf5::transaction
