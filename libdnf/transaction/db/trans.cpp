/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "trans.hpp"

#include "db.hpp"

#include "libdnf/transaction/transaction.hpp"


namespace libdnf::transaction {


static constexpr const char * select_sql = R"**(
    SELECT
        "trans"."id",
        "dt_begin",
        "dt_end",
        "rpmdb_version_begin",
        "rpmdb_version_end",
        "releasever",
        "user_id",
        "description",
        "comment",
        "trans_state"."name" AS "state"
    FROM "trans"
    LEFT JOIN "trans_state" ON "trans"."state_id" = "trans_state"."id"
)**";


std::vector<int64_t> TransactionDbUtils::select_transaction_ids(const BaseWeakPtr & base) {
    auto conn = transaction_db_connect(*base);

    auto query = libdnf::utils::SQLite3::Query(*conn, "SELECT \"id\" FROM \"trans\" ORDER BY \"id\"");

    std::vector<int64_t> res;

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        res.push_back(query.get<int64_t>("id"));
    }

    return res;
}


std::vector<Transaction> TransactionDbUtils::load_from_select(
    const BaseWeakPtr & base, libdnf::utils::SQLite3::Query & query) {
    std::vector<Transaction> res;

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto & trans = res.emplace_back(base, query.get<int>("id"));
        trans.set_dt_start(query.get<int64_t>("dt_begin"));
        trans.set_dt_end(query.get<int64_t>("dt_end"));
        trans.set_rpmdb_version_begin(query.get<std::string>("rpmdb_version_begin"));
        trans.set_rpmdb_version_end(query.get<std::string>("rpmdb_version_end"));
        trans.set_releasever(query.get<std::string>("releasever"));
        trans.set_user_id(query.get<uint32_t>("user_id"));
        trans.set_description(query.get<std::string>("description"));
        trans.set_comment(query.get<std::string>("comment"));
        trans.set_state(transaction_state_from_string(query.get<std::string>("state")));
    }

    return res;
}


std::vector<Transaction> TransactionDbUtils::select_transactions_by_ids(
    const BaseWeakPtr & base, const std::vector<int64_t> & ids) {
    auto conn = transaction_db_connect(*base);

    std::string sql = select_sql;

    if (!ids.empty()) {
        sql += " WHERE \"trans\".\"id\" IN (";
        for (size_t i = 0; i < ids.size(); ++i) {
            if (i == 0) {
                sql += "?";
            } else {
                sql += ", ?";
            }
        }
        sql += ")";
    }

    auto query = libdnf::utils::SQLite3::Query(*conn, sql);

    for (size_t i = 0; i < ids.size(); ++i) {
        query.bind(static_cast<int>(i + 1), ids[i]);
    }

    return TransactionDbUtils::load_from_select(base, query);
}


std::vector<Transaction> TransactionDbUtils::select_transactions_by_range(
    const BaseWeakPtr & base, int64_t start, int64_t end) {
    auto conn = transaction_db_connect(*base);

    std::string sql = std::string(select_sql) + " WHERE \"trans\".\"id\" >= ? AND \"trans\".\"id\" <= ?";

    auto query = libdnf::utils::SQLite3::Query(*conn, sql);
    query.bindv(start, end);

    return TransactionDbUtils::load_from_select(base, query);
}


static constexpr const char * SQL_TRANS_INSERT = R"**(
    INSERT INTO
        "trans" (
            "dt_begin",
            "dt_end",
            "rpmdb_version_begin",
            "rpmdb_version_end",
            "releasever",
            "user_id",
            "description",
            "comment",
            "state_id",
            "id"
        )
        VALUES
            (?, ?, ?, ?, ?, ?, ?, ?, (SELECT "id" FROM "trans_state" WHERE "name" = ?), ?)
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> TransactionDbUtils::trans_insert_new_query(
    libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_TRANS_INSERT);
    return query;
}


void TransactionDbUtils::trans_insert(libdnf::utils::SQLite3::Statement & query, Transaction & trans) {
    query.bindv(
        trans.get_dt_start(),
        trans.get_dt_end(),
        trans.get_rpmdb_version_begin(),
        trans.get_rpmdb_version_end(),
        trans.get_releasever(),
        trans.get_user_id(),
        trans.get_description(),
        trans.get_comment(),
        transaction_state_to_string(trans.get_state()));

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


static constexpr const char * SQL_TRANS_UPDATE = R"**(
    UPDATE
        "trans"
    SET
        "dt_begin" = ?,
        "dt_end" = ?,
        "rpmdb_version_begin" = ?,
        "rpmdb_version_end" = ?,
        "releasever" = ?,
        "user_id" = ?,
        "description" = ?,
        "comment" = ?,
        "state_id" = (SELECT "id" FROM "trans_state" WHERE "name" = ?)
    WHERE
        "id" = ?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> TransactionDbUtils::trans_update_new_query(
    libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_TRANS_UPDATE);
    return query;
}


void TransactionDbUtils::trans_update(libdnf::utils::SQLite3::Statement & query, Transaction & trans) {
    query.bindv(
        // SET key=value
        trans.get_dt_start(),
        trans.get_dt_end(),
        trans.get_rpmdb_version_begin(),
        trans.get_rpmdb_version_end(),
        trans.get_releasever(),
        trans.get_user_id(),
        trans.get_description(),
        trans.get_comment(),
        transaction_state_to_string(trans.get_state()),
        // WHERE id=?
        trans.get_id());
    query.step();
    query.reset();
}


}  // namespace libdnf::transaction
