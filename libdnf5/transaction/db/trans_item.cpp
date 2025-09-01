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


#include "trans_item.hpp"

#include "repo.hpp"

#include "libdnf5/transaction/transaction.hpp"
#include "libdnf5/transaction/transaction_item.hpp"


namespace libdnf5::transaction {


void TransItemDbUtils::transaction_item_select(libdnf5::utils::SQLite3::Query & query, TransactionItem & ti) {
    ti.set_id(query.get<int64_t>("id"));
    ti.set_action(transaction_item_action_from_string(query.get<std::string>("action")));
    ti.set_reason(transaction_item_reason_from_string(query.get<std::string>("reason")));
    ti.set_state(transaction_item_state_from_string(query.get<std::string>("state")));
    ti.set_repoid(query.get<std::string>("repoid"));
    ti.set_item_id(query.get<int64_t>("item_id"));
}


static constexpr const char * SQL_TRANS_ITEM_INSERT = R"**(
    INSERT INTO
        "trans_item" (
            "id",
            "trans_id",
            "item_id",
            "repo_id",
            "action_id",
            "reason_id",
            "state_id"
        )
    VALUES (
        null,
        ?,
        ?,
        ?,
        (SELECT "id" FROM "trans_item_action" WHERE "name" = ?),
        (SELECT "id" FROM "trans_item_reason" WHERE "name" = ?),
        (SELECT "id" FROM "trans_item_state" WHERE "name" = ?)
    )
)**";


std::unique_ptr<libdnf5::utils::SQLite3::Statement> TransItemDbUtils::trans_item_insert_new_query(
    libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_TRANS_ITEM_INSERT);
    return query;
}


int64_t TransItemDbUtils::transaction_item_insert(libdnf5::utils::SQLite3::Statement & query, TransactionItem & ti) {
    // try to find an existing repo
    auto query_repo_select_pkg = repo_select_pk_new_query(query.get_db());
    auto repo_id = repo_select_pk(*query_repo_select_pkg, ti.get_repoid());

    if (!repo_id) {
        // if an existing repo was not found, insert a new record
        auto query_repo_insert = repo_insert_new_query(query.get_db());
        repo_id = repo_insert(*query_repo_insert, ti.get_repoid());
    }

    // save the transaction item
    query.bindv(
        ti.get_transaction().get_id(),
        ti.get_item_id(),
        repo_id,
        transaction_item_action_to_string(ti.get_action()),
        transaction_item_reason_to_string(ti.get_reason()),
        transaction_item_state_to_string(ti.get_state()));
    query.step();
    auto pk = query.last_insert_rowid();
    ti.set_id(pk);
    query.reset();
    return pk;
}


}  // namespace libdnf5::transaction
