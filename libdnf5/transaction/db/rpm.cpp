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


#include "rpm.hpp"

#include "arch.hpp"
#include "item.hpp"
#include "pkg_name.hpp"
#include "trans_item.hpp"

#include "libdnf5/transaction/rpm_package.hpp"
#include "libdnf5/transaction/transaction.hpp"


namespace libdnf5::transaction {


static constexpr const char * SQL_RPM_TRANSACTION_ITEM_SELECT = R"**(
    SELECT
        "ti"."id",
        "trans_item_action"."name" AS "action",
        "trans_item_reason"."name" AS "reason",
        "trans_item_state"."name" AS "state",
        "r"."repoid",
        "i"."item_id",
        "pkg_name"."name",
        "i"."epoch",
        "i"."version",
        "i"."release",
        "arch"."name" AS "arch"
    FROM "trans_item" "ti"
    JOIN "repo" "r" ON "ti"."repo_id" = "r"."id"
    JOIN "rpm" "i" USING ("item_id")
    LEFT JOIN "trans_item_action" ON "ti"."action_id" = "trans_item_action"."id"
    LEFT JOIN "trans_item_reason" ON "ti"."reason_id" = "trans_item_reason"."id"
    LEFT JOIN "trans_item_state" ON "ti"."state_id" = "trans_item_state"."id"
    LEFT JOIN "pkg_name" ON "i"."name_id" = "pkg_name"."id"
    LEFT JOIN "arch" ON "i"."arch_id" = "arch"."id"
    WHERE "ti"."trans_id" = ?
)**";


// Create a query that returns all rpm transaction items for a transaction
static std::unique_ptr<libdnf5::utils::SQLite3::Query> rpm_transaction_item_select_new_query(
    libdnf5::utils::SQLite3 & conn, int64_t transaction_id) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Query>(conn, SQL_RPM_TRANSACTION_ITEM_SELECT);
    query->bindv(transaction_id);
    return query;
}


int64_t RpmDbUtils::rpm_transaction_item_select(libdnf5::utils::SQLite3::Query & query, Package & pkg) {
    TransItemDbUtils::transaction_item_select(query, pkg);
    pkg.set_name(query.get<std::string>("name"));
    pkg.set_epoch(std::to_string(query.get<uint32_t>("epoch")));
    pkg.set_version(query.get<std::string>("version"));
    pkg.set_release(query.get<std::string>("release"));
    pkg.set_arch(query.get<std::string>("arch"));
    return pkg.get_id();
}


static constexpr const char * SQL_RPM_INSERT = R"**(
    INSERT INTO
        "rpm" (
            "item_id",
            "name_id",
            "epoch",
            "version",
            "release",
            "arch_id"
        )
    VALUES
        (?, (SELECT "id" FROM "pkg_name" WHERE "name" = ?), ?, ?, ?, (SELECT "id" FROM "arch" WHERE "name" = ?))
)**";


// Create a query (statement) that inserts new records to the 'rpm' table
static std::unique_ptr<libdnf5::utils::SQLite3::Statement> rpm_insert_new_query(libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_RPM_INSERT);
    return query;
}


int64_t RpmDbUtils::rpm_insert(libdnf5::utils::SQLite3::Statement & query, const Package & rpm) {
    query.bindv(
        rpm.get_item_id(), rpm.get_name(), rpm.get_epoch_int(), rpm.get_version(), rpm.get_release(), rpm.get_arch());
    query.step();
    int64_t result = query.last_insert_rowid();
    query.reset();
    return result;
}


static constexpr const char * SQL_RPM_SELECT_PK = R"**(
    SELECT
        "item_id"
    FROM
        "rpm"
    LEFT JOIN "pkg_name" ON "rpm"."name_id" = "pkg_name"."id"
    LEFT JOIN "arch" ON "rpm"."arch_id" = "arch"."id"
    WHERE
        "pkg_name"."name" = ?
        AND "epoch" = ?
        AND "version" = ?
        AND "release" = ?
        AND "arch"."name" = ?
)**";


// Create a query that returns primary keys from table 'rpm'
static std::unique_ptr<libdnf5::utils::SQLite3::Statement> rpm_select_pk_new_query(libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_RPM_SELECT_PK);
    return query;
}


int64_t RpmDbUtils::rpm_select_pk(libdnf5::utils::SQLite3::Statement & query, const Package & rpm) {
    query.bindv(rpm.get_name(), rpm.get_epoch_int(), rpm.get_version(), rpm.get_release(), rpm.get_arch());

    int64_t result = 0;
    if (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        result = query.get<int64_t>(0);
    }
    query.reset();
    return result;
}


bool RpmDbUtils::rpm_select(libdnf5::utils::SQLite3::Query & query, int64_t rpm_id, Package & rpm) {
    bool result = false;
    query.bindv(rpm_id);

    if (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        rpm.set_item_id(query.get<int>("item_id"));
        rpm.set_name(query.get<std::string>("name"));
        rpm.set_epoch(std::to_string(query.get<uint32_t>("epoch")));
        rpm.set_version(query.get<std::string>("version"));
        rpm.set_release(query.get<std::string>("release"));
        rpm.set_arch(query.get<std::string>("arch"));
        result = true;
    }

    query.reset();
    return result;
}


std::vector<Package> RpmDbUtils::get_transaction_packages(libdnf5::utils::SQLite3 & conn, Transaction & trans) {
    std::vector<Package> result;

    auto query = rpm_transaction_item_select_new_query(conn, trans.get_id());
    while (query->step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        Package trans_item(trans);
        rpm_transaction_item_select(*query, trans_item);
        result.push_back(std::move(trans_item));
    }

    return result;
}


void RpmDbUtils::insert_transaction_packages(libdnf5::utils::SQLite3 & conn, Transaction & trans) {
    auto query_rpm_select_pk = rpm_select_pk_new_query(conn);
    auto query_item_insert = item_insert_new_query(conn);
    auto query_pkg_name_insert_if_not_exists = pkg_name_insert_if_not_exists_new_query(conn);
    auto query_arch_insert_if_not_exists = arch_insert_if_not_exists_new_query(conn);
    auto query_rpm_insert = rpm_insert_new_query(conn);
    auto query_trans_item_insert = TransItemDbUtils::trans_item_insert_new_query(conn);

    for (auto & pkg : trans.get_packages()) {
        pkg.set_item_id(rpm_select_pk(*query_rpm_select_pk, pkg));
        if (pkg.get_item_id() == 0) {
            // insert into 'item' table, create item_id
            pkg.set_item_id(item_insert(*query_item_insert));
            // insert package name into 'pkg_name' table if not exists
            pkg_name_insert_if_not_exists(*query_pkg_name_insert_if_not_exists, pkg.get_name());
            // insert arch name into 'arch' table if not exists
            arch_insert_if_not_exists(*query_arch_insert_if_not_exists, pkg.get_arch());
            // insert into 'rpm' table
            rpm_insert(*query_rpm_insert, pkg);
        }
        TransItemDbUtils::transaction_item_insert(*query_trans_item_insert, pkg);
    }
}


}  // namespace libdnf5::transaction
