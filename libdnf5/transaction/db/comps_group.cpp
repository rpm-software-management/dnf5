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


#include "comps_group.hpp"

#include "comps_group_package.hpp"
#include "item.hpp"
#include "trans_item.hpp"

#include "libdnf5/comps/group/package.hpp"
#include "libdnf5/transaction/comps_group.hpp"
#include "libdnf5/transaction/transaction.hpp"
#include "libdnf5/transaction/transaction_item.hpp"


namespace libdnf5::transaction {


static constexpr const char * SQL_COMPS_GROUP_TRANSACTION_ITEM_SELECT = R"**(
    SELECT
        "ti"."id",
        "trans_item_action"."name" AS "action",
        "trans_item_reason"."name" AS "reason",
        "trans_item_state"."name" AS "state",
        "r"."repoid",
        "i"."item_id",
        "i"."groupid",
        "i"."name",
        "i"."translated_name",
        "i"."pkg_types"
    FROM "trans_item" "ti"
    JOIN "repo" "r" ON "ti"."repo_id" = "r"."id"
    JOIN "comps_group" "i" USING ("item_id")
    LEFT JOIN "trans_item_action" ON "ti"."action_id" = "trans_item_action"."id"
    LEFT JOIN "trans_item_reason" ON "ti"."reason_id" = "trans_item_reason"."id"
    LEFT JOIN "trans_item_state" ON "ti"."state_id" = "trans_item_state"."id"
    WHERE "ti"."trans_id" = ?
)**";


static std::unique_ptr<libdnf5::utils::SQLite3::Query> comps_group_transaction_item_select_new_query(
    libdnf5::utils::SQLite3 & conn, int64_t transaction_id) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Query>(conn, SQL_COMPS_GROUP_TRANSACTION_ITEM_SELECT);
    query->bindv(transaction_id);
    return query;
}


std::vector<CompsGroup> CompsGroupDbUtils::get_transaction_comps_groups(
    libdnf5::utils::SQLite3 & conn, Transaction & trans) {
    std::vector<CompsGroup> result;

    auto query = comps_group_transaction_item_select_new_query(conn, trans.get_id());

    while (query->step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        CompsGroup ti(trans);
        TransItemDbUtils::transaction_item_select(*query, ti);
        //        auto trans_item = std::make_shared< TransactionItem >(trans);
        //        auto item = std::make_shared< CompsGroup >(trans);
        //        trans_item->setItem(item);
        ti.set_group_id(query->get<std::string>("groupid"));
        ti.set_name(query->get<std::string>("name"));
        ti.set_translated_name(query->get<std::string>("translated_name"));
        ti.set_package_types(static_cast<comps::PackageType>(query->get<int>("pkg_types")));
        CompsGroupPackageDbUtils::comps_group_packages_select(conn, ti);
        result.push_back(std::move(ti));
    }

    return result;
}


static constexpr const char * SQL_COMPS_GROUP_INSERT = R"**(
    INSERT INTO
        "comps_group" (
            "item_id",
            "groupid",
            "name",
            "translated_name",
            "pkg_types"
        )
    VALUES
        (?, ?, ?, ?, ?)
)**";


// Create a query (statement) that inserts new records to the 'comps_group' table
static std::unique_ptr<libdnf5::utils::SQLite3::Statement> comps_group_insert_new_query(
    libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_COMPS_GROUP_INSERT);
    return query;
}


int64_t CompsGroupDbUtils::comps_group_insert(libdnf5::utils::SQLite3::Statement & query, CompsGroup & grp) {
    // insert a record to the 'item' table first
    auto query_item_insert = item_insert_new_query(query.get_db());
    auto item_id = item_insert(*query_item_insert);

    query.bindv(
        item_id,
        grp.get_group_id(),
        grp.get_name(),
        grp.get_translated_name(),
        static_cast<int>(grp.get_package_types()));
    query.step();
    query.reset();
    grp.set_item_id(item_id);
    return item_id;
}


void CompsGroupDbUtils::insert_transaction_comps_groups(libdnf5::utils::SQLite3 & conn, Transaction & trans) {
    auto query_comps_group_insert = comps_group_insert_new_query(conn);
    auto query_trans_item_insert = TransItemDbUtils::trans_item_insert_new_query(conn);

    for (auto & grp : trans.get_comps_groups()) {
        comps_group_insert(*query_comps_group_insert, grp);
        TransItemDbUtils::transaction_item_insert(*query_trans_item_insert, grp);
        CompsGroupPackageDbUtils::comps_group_packages_insert(conn, grp);
    }
}


}  // namespace libdnf5::transaction
