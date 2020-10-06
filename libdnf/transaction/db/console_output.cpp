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


#include "console_output.hpp"

#include "libdnf/transaction/transaction.hpp"


namespace libdnf::transaction {


const char * SQL_CONSOLE_OUTPUT_INSERT = R"**(
    INSERT INTO
        console_output (
            trans_id,
            file_descriptor,
            line
        )
    VALUES
        (?, ?, ?);
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> console_output_insert_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_CONSOLE_OUTPUT_INSERT);
    return query;
}


int64_t console_output_insert_line(libdnf::utils::SQLite3 & conn, Transaction & trans, int file_descriptor, const std::string & line) {
    auto query = console_output_insert_new_query(conn);

    query->bindv(
        trans.get_id(),
        file_descriptor,
        line
    );
    query->step();
    return query->last_insert_rowid();
}


const char * SQL_CONSOLE_OUTPUT_SELECT = R"**(
    SELECT
        file_descriptor,
        line
    FROM
        console_output
    WHERE
        trans_id = ?
    ORDER BY
        id
)**";


std::unique_ptr<libdnf::utils::SQLite3::Query> console_output_select_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_CONSOLE_OUTPUT_SELECT);
    return query;
}


std::vector<std::pair<int, std::string>> console_output_load(libdnf::utils::SQLite3 & conn, Transaction & trans) {
    std::vector<std::pair<int, std::string>> result;

    auto query = console_output_select_new_query(conn);
    query->bindv(trans.get_id());
    while (query->step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto file_descriptor = query->get<int>("file_descriptor");
        auto line = query->get<std::string>("line");
        result.emplace_back(file_descriptor, line);
    }

    return result;
}


}  // namespace libdnf::transaction
