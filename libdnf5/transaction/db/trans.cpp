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


#include "trans.hpp"

#include "db.hpp"

#include "libdnf5/transaction/transaction.hpp"


namespace libdnf5::transaction {


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

    auto query = libdnf5::utils::SQLite3::Query(*conn, "SELECT \"id\" FROM \"trans\" ORDER BY \"id\"");

    std::vector<int64_t> res;

    while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        res.push_back(query.get<int64_t>("id"));
    }

    return res;
}


std::vector<Transaction> TransactionDbUtils::load_from_select(
    const BaseWeakPtr & base, libdnf5::utils::SQLite3::Query & query) {
    std::vector<Transaction> res;

    while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        Transaction trans(base, query.get<int>("id"));
        trans.set_dt_start(query.get<int64_t>("dt_begin"));
        trans.set_dt_end(query.get<int64_t>("dt_end"));
        trans.set_rpmdb_version_begin(query.get<std::string>("rpmdb_version_begin"));
        trans.set_rpmdb_version_end(query.get<std::string>("rpmdb_version_end"));
        trans.set_releasever(query.get<std::string>("releasever"));
        trans.set_user_id(query.get<uint32_t>("user_id"));
        trans.set_description(query.get<std::string>("description"));
        trans.set_comment(query.get<std::string>("comment"));
        trans.set_state(transaction_state_from_string(query.get<std::string>("state")));
        res.emplace_back(std::move(trans));
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

    auto query = libdnf5::utils::SQLite3::Query(*conn, sql);

    for (size_t i = 0; i < ids.size(); ++i) {
        query.bind(static_cast<int>(i + 1), ids[i]);
    }

    return TransactionDbUtils::load_from_select(base, query);
}


std::vector<Transaction> TransactionDbUtils::select_transactions_by_range(
    const BaseWeakPtr & base, int64_t start, int64_t end) {
    auto conn = transaction_db_connect(*base);

    std::string sql = std::string(select_sql) + " WHERE \"trans\".\"id\" >= ? AND \"trans\".\"id\" <= ?";

    auto query = libdnf5::utils::SQLite3::Query(*conn, sql);
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


std::unique_ptr<libdnf5::utils::SQLite3::Statement> TransactionDbUtils::trans_insert_new_query(
    libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_TRANS_INSERT);
    return query;
}


void TransactionDbUtils::trans_insert(libdnf5::utils::SQLite3::Statement & query, Transaction & trans) {
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


std::unique_ptr<libdnf5::utils::SQLite3::Statement> TransactionDbUtils::trans_update_new_query(
    libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_TRANS_UPDATE);
    return query;
}


void TransactionDbUtils::trans_update(libdnf5::utils::SQLite3::Statement & query, Transaction & trans) {
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

static constexpr const char * SQL_TRANS_ITEM_NAME_ARCH_REASON = R"**(
    SELECT
        "ti"."reason_id"
    FROM
        "trans_item" "ti"
    JOIN
        "trans" "t" ON ("ti"."trans_id" = "t"."id")
    JOIN
        "rpm" "i" USING ("item_id")
    JOIN
        "pkg_name" ON "i"."name_id" = "pkg_name"."id"
    JOIN
        "arch" ON "i"."arch_id" = "arch"."id"
    WHERE
        "t"."state_id" = 2
        AND "ti"."action_id" NOT IN (6)
        AND "pkg_name"."name" = ?
        AND "arch"."name" = ?
        AND "ti"."trans_id" <= ?
    ORDER BY
        "ti"."trans_id" DESC
    LIMIT 1
)**";

TransactionItemReason TransactionDbUtils::transaction_item_reason_at(
    const BaseWeakPtr & base, const std::string & name, const std::string & arch, int64_t transaction_id_point) {
    auto conn = transaction_db_connect(*base);

    auto query = libdnf5::utils::SQLite3::Query(*conn, SQL_TRANS_ITEM_NAME_ARCH_REASON);
    query.bindv(name, arch, transaction_id_point);

    if (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        auto reason = static_cast<transaction::TransactionItemReason>(query.get<int>("reason_id"));
        return reason;
    }

    return TransactionItemReason::NONE;
}

static constexpr const char * ITEM_COUNT_SQL = R"**(
    SELECT
        "trans"."id",
        COUNT("trans_item"."trans_id") AS "item_count"
    FROM "trans"
    LEFT JOIN "trans_item" ON "trans"."id" = "trans_item"."trans_id"
)**";

std::unordered_map<int64_t, int64_t> TransactionDbUtils::transactions_item_counts(
    const BaseWeakPtr & base, const std::vector<Transaction> & transactions) {
    auto conn = transaction_db_connect(*base);

    std::string sql = ITEM_COUNT_SQL;

    if (!transactions.empty()) {
        sql += " WHERE \"trans\".\"id\" IN (";
        for (size_t i = 0; i < transactions.size(); ++i) {
            if (i == 0) {
                sql += "?";
            } else {
                sql += ", ?";
            }
        }
        sql += ")";
    }

    // GROUP BY has to be after WHERE clause
    sql += "GROUP BY \"trans\".\"id\"";

    auto query = libdnf5::utils::SQLite3::Query(*conn, sql);

    for (size_t i = 0; i < transactions.size(); ++i) {
        query.bind(static_cast<int>(i + 1), transactions[i].get_id());
    }

    std::unordered_map<int64_t, int64_t> id_to_count;

    while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        id_to_count.emplace(query.get<int>("id"), query.get<int64_t>("item_count"));
    }

    return id_to_count;
}

static constexpr const char * SQL_TRANS_CONTAINS_RPM_NAME = R"**(
    SELECT DISTINCT
        "t"."id"
    FROM
        "trans_item" "ti"
    JOIN
        "trans" "t" ON ("ti"."trans_id" = "t"."id")
    JOIN
        "rpm" "i" USING ("item_id")
    JOIN
        "pkg_name" ON "i"."name_id" = "pkg_name"."id"
)**";

void TransactionDbUtils::filter_transactions_by_pkg_names(
    const BaseWeakPtr & base, std::vector<Transaction> & transactions, const std::vector<std::string> & pkg_names) {
    libdnf_assert(!pkg_names.empty(), "Cannot filter transactions, no package names provided.");

    auto conn = transaction_db_connect(*base);

    std::string sql = SQL_TRANS_CONTAINS_RPM_NAME;

    sql += "WHERE (\"pkg_name\".\"name\" GLOB ?";
    for (size_t i = 1; i < pkg_names.size(); i++) {
        sql += "OR \"pkg_name\".\"name\" GLOB ?";
    }
    sql += ")";

    // Limit to only selected transactions
    if (!transactions.empty()) {
        sql += "AND \"t\".\"id\" IN (";
        for (size_t i = 0; i < transactions.size(); ++i) {
            if (i == 0) {
                sql += "?";
            } else {
                sql += ", ?";
            }
        }
        sql += ")";
    }

    auto query = libdnf5::utils::SQLite3::Query(*conn, sql);

    for (size_t i = 0; i < pkg_names.size(); ++i) {
        // bind indexes from 1
        query.bind(static_cast<int>(i + 1), pkg_names[i]);
    }

    for (size_t i = 0; i < transactions.size(); ++i) {
        query.bind(static_cast<int>(i + pkg_names.size() + 1), transactions[i].get_id());
    }

    std::vector<int> ids_to_keep;
    while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        ids_to_keep.push_back(query.get<int>("id"));
    }

    transactions.erase(
        std::remove_if(
            transactions.begin(),
            transactions.end(),
            [&ids_to_keep](auto trans) {
                return std::find(ids_to_keep.begin(), ids_to_keep.end(), trans.get_id()) == ids_to_keep.end();
            }),
        transactions.end());
}

}  // namespace libdnf5::transaction
