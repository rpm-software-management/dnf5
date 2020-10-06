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
#include "trans_item.hpp"

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


std::vector<std::unique_ptr<CompsGroup>> get_transaction_comps_groups(libdnf::utils::SQLite3 & conn, Transaction & trans) {
    std::vector<std::unique_ptr<CompsGroup>> result;

    auto query = comps_group_transaction_item_select_new_query(conn, trans.get_id());

    while (query->step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto ti = std::make_unique<CompsGroup>(trans);
        transaction_item_select(*query, *ti);
//        auto trans_item = std::make_shared< TransactionItem >(trans);
//        auto item = std::make_shared< CompsGroup >(trans);
//        trans_item->setItem(item);
        ti->set_group_id(query->get<std::string>("groupid"));
        ti->set_name(query->get<std::string>("name"));
        ti->set_translated_name(query->get<std::string>("translated_name"));
        ti->set_package_types(static_cast<CompsPackageType>(query->get<int>("pkg_types")));
        comps_group_packages_select(conn, *ti);
        result.push_back(std::move(ti));
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
    auto query_item_insert = item_insert_new_query(query.get_db(), TransactionItemType::GROUP);
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
    grp.set_item_id(item_id);
    return item_id;
}


void insert_transaction_comps_groups(libdnf::utils::SQLite3 & conn, Transaction & trans) {
    auto query_comps_group_insert = comps_group_insert_new_query(conn);
    auto query_trans_item_insert = trans_item_insert_new_query(conn);

    for (auto & grp : trans.get_comps_groups()) {
        comps_group_insert(*query_comps_group_insert, *grp);
        transaction_item_insert(*query_trans_item_insert, *grp);
        comps_group_packages_insert(conn, *grp);
    }
}


}  // namespace libdnf::transaction
