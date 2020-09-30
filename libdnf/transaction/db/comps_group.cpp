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


#include "comps_group.hpp"
#include "comps_group_package.hpp"
#include "item.hpp"

#include "libdnf/transaction/comps_group.hpp"
#include "libdnf/transaction/transaction.hpp"
#include "libdnf/transaction/transaction_item.hpp"


namespace libdnf::transaction {


static const char *SQL_COMPS_GROUP_TRANSACTION_ITEM_SELECT = R"**(
    SELECT
        /* trans_item */
        ti.id,
        ti.action,
        ti.reason,
        ti.state,
        /* repo */
        r.repoid,
        /* comps_group */
        i.item_id,
        i.groupid,
        i.name,
        i.translated_name,
        i.pkg_types
    FROM
        trans_item ti
    JOIN
        repo r ON ti.repo_id == r.id
    JOIN
        comps_group i USING (item_id)
    WHERE
        ti.trans_id = ?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Query> comps_group_transaction_item_select_new_query(libdnf::utils::SQLite3 & conn, int64_t transaction_id) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_COMPS_GROUP_TRANSACTION_ITEM_SELECT);
    query->bindv(transaction_id);
    return query;
}


std::vector<std::shared_ptr<TransactionItem>> get_transaction_comps_groups(Transaction & trans) {
    std::vector<std::shared_ptr<TransactionItem>> result;

    auto query = comps_group_transaction_item_select_new_query(trans.get_connection(), trans.get_id());

    while (query->step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto trans_item = std::make_shared< TransactionItem >(trans);
        auto item = std::make_shared< CompsGroup >(trans);
        trans_item->setItem(item);

        trans_item->set_id(query->get<int64_t>("id"));
        trans_item->set_action(static_cast<TransactionItemAction>(query->get<int>("action")));
        trans_item->set_reason(static_cast<TransactionItemReason>(query->get<int>("reason")));
        trans_item->set_state(static_cast<TransactionItemState>(query->get<int>("state")));
        item->setId(query->get<int64_t>("item_id"));
        item->set_group_id(query->get<std::string>("groupid"));
        item->set_name(query->get<std::string>("name"));
        item->set_translated_name(query->get<std::string>("translated_name"));
        item->set_package_types(static_cast<CompsPackageType>(query->get<int>("pkg_types")));
        comps_group_packages_select(*item);

        result.push_back(trans_item);
    }

    return result;
}


static const char * SQL_COMPS_GROUP_INSERT = R"**(
    INSERT INTO
        comps_group (
            item_id,
            groupid,
            name,
            translated_name,
            pkg_types
        )
    VALUES
        (?, ?, ?, ?, ?)
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> comps_group_insert_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_COMPS_GROUP_INSERT);
    return query;
}


int64_t comps_group_insert(libdnf::utils::SQLite3::Statement & query, CompsGroup & grp) {
    // insert a record to the 'item' table first
    auto query_item_insert = item_insert_new_query(grp.get_transaction().get_connection(), TransactionItemType::GROUP);
    auto item_id = item_insert(*query_item_insert);

    query.bindv(
        item_id,
        grp.get_group_id(),
        grp.get_name(),
        grp.get_translated_name(),
        static_cast<int>(grp.get_package_types())
    );
    query.step();
    query.reset();
    grp.setId(item_id);
    return item_id;
}


}  // namespace libdnf::transaction
