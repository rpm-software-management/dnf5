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


#include "trans_item.hpp"

#include "repo.hpp"

#include "libdnf/transaction/transaction.hpp"
#include "libdnf/transaction/transaction_item.hpp"


namespace libdnf::transaction {


void transaction_item_select(libdnf::utils::SQLite3::Query & query, TransactionItem & ti) {
    ti.set_id(query.get<int64_t>("id"));
    ti.set_action(static_cast<TransactionItemAction>(query.get<int>("action")));
    ti.set_reason(static_cast<TransactionItemReason>(query.get<int>("reason")));
    ti.set_state(static_cast<TransactionItemState>(query.get<int>("state")));
    ti.set_item_id(query.get<int64_t>("item_id"));
}


static const char * SQL_TRANS_ITEM_INSERT = R"**(
    INSERT INTO
        trans_item (
            id,
            trans_id,
            item_id,
            repo_id,
            action,
            reason,
            state
        )
    VALUES
        (null, ?, ?, ?, ?, ?, ?)
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> trans_item_insert_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_TRANS_ITEM_INSERT);
    return query;
}


int64_t transaction_item_insert(libdnf::utils::SQLite3::Statement & query, TransactionItem & ti) {
    auto & conn = ti.get_transaction().get_connection();

    // try to find an existing repo
    auto query_repo_select_pkg = repo_select_pk_new_query(conn);
    auto repo_id = repo_select_pk(*query_repo_select_pkg, ti.get_repoid());

    if (!repo_id) {
        // if an existing repo was not found, insert a new record
        auto query_repo_insert = repo_insert_new_query(conn);
        repo_id = repo_insert(*query_repo_insert, ti.get_repoid());
    }

    // save the transaction item
    query.bindv(
        ti.get_transaction().get_id(),
        ti.get_item_id(),
        repo_id,
        static_cast<int>(ti.get_action()),
        static_cast<int>(ti.get_reason()),
        static_cast<int>(ti.get_state()));
    query.step();
    auto pk = query.last_insert_rowid();
    ti.set_id(pk);
    query.reset();
    return pk;
}


}  // namespace libdnf::transaction
