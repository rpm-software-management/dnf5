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


#include "trans.hpp"

#include "libdnf/transaction/transaction.hpp"


namespace libdnf::transaction {


const char * SQL_TRANS_SELECT = R"**(
    SELECT
        id,
        dt_begin,
        dt_end,
        rpmdb_version_begin,
        rpmdb_version_end,
        releasever,
        user_id,
        cmdline,
        state
    FROM
        trans
    WHERE
        id = ?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Query> trans_select_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_TRANS_SELECT);
    return query;
}


bool trans_select(libdnf::utils::SQLite3::Query & query, int64_t transaction_id, Transaction & trans) {
    bool result = false;
    query.bindv(transaction_id);

    if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        trans.set_id(query.get<int>("id"));
        trans.set_dt_begin(query.get<int64_t>("dt_begin"));
        trans.set_dt_end(query.get<int64_t>("dt_end"));
        trans.set_rpmdb_version_begin(query.get<std::string>("rpmdb_version_begin"));
        trans.set_rpmdb_version_end(query.get<std::string>("rpmdb_version_end"));
        trans.set_releasever(query.get<std::string>("releasever"));
        trans.set_user_id(query.get<uint32_t>("user_id"));
        trans.set_cmdline(query.get<std::string>("cmdline"));
        trans.set_state(static_cast<TransactionState>(query.get<int>("state")));
        result = true;
    }

    query.reset();
    return result;
}


static const char * SQL_TRANS_INSERT = R"**(
    INSERT INTO
        trans (
            dt_begin,
            dt_end,
            rpmdb_version_begin,
            rpmdb_version_end,
            releasever,
            user_id,
            cmdline,
            state,
            id
        )
        VALUES
            (?, ?, ?, ?, ?, ?, ?, ?, ?)
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> trans_insert_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_TRANS_INSERT);
    return query;
}


void trans_insert(libdnf::utils::SQLite3::Statement & query, Transaction & trans) {
    query.bindv(
        trans.get_dt_begin(),
        trans.get_dt_end(),
        trans.get_rpmdb_version_begin(),
        trans.get_rpmdb_version_end(),
        trans.get_releasever(),
        trans.get_user_id(),
        trans.get_cmdline(),
        static_cast<int>(trans.get_state())
    );

    if (trans.get_id() > 0) {
        // use an existing primary key
        query.bind(9, trans.get_id());
        query.step();
    } else {
        // no primary key specified, retrieve a new one
        query.step();
        trans.set_id(query.last_insert_rowid());
    }
    query.reset();
}


static const char * SQL_TRANS_UPDATE = R"**(
    UPDATE
        trans
    SET
        dt_begin=?,
        dt_end=?,
        rpmdb_version_begin=?,
        rpmdb_version_end=?,
        releasever=?,
        user_id=?,
        cmdline=?,
        state=?
    WHERE
        id = ?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> trans_update_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_TRANS_UPDATE);
    return query;
}


void trans_update(libdnf::utils::SQLite3::Statement & query, const Transaction & trans) {
    query.bindv(
        // SET key=value
        trans.get_dt_begin(),
        trans.get_dt_end(),
        trans.get_rpmdb_version_begin(),
        trans.get_rpmdb_version_end(),
        trans.get_releasever(),
        trans.get_user_id(),
        trans.get_cmdline(),
        static_cast<int>(trans.get_state()),
        // WHERE id=?
        trans.get_id()
    );
    query.step();
    query.reset();
}


}  // namespace libdnf::transaction
