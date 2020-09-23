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


#include "rpm.hpp"

#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/transaction.hpp"


namespace libdnf::transaction {


static const char *SQL_RPM_TRANSACTION_ITEM_SELECT = R"**(
    SELECT
        /* trans_item */
        ti.id,
        ti.action,
        ti.reason,
        ti.state,
        /* repo */
        r.repoid,
        /* rpm */
        i.item_id,
        i.name,
        i.epoch,
        i.version,
        i.release,
        i.arch
    FROM
        trans_item ti
    JOIN
        repo r ON ti.repo_id == r.id
    JOIN
        rpm i USING (item_id)
    WHERE
        ti.trans_id = ?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Query> rpm_transaction_item_select_new_query(libdnf::utils::SQLite3 & conn, int64_t transaction_id) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_RPM_TRANSACTION_ITEM_SELECT);
    query->bindv(transaction_id);
    return query;
}


int64_t rpm_transaction_item_select(libdnf::utils::SQLite3::Query & query, TransactionItem & ti) {
    auto item = std::make_shared<Package>(ti.get_trans());
    ti.setItem(item);
    ti.set_id(query.get< int >("id"));
    ti.set_action(static_cast< TransactionItemAction >(query.get< int >("action")));
    ti.set_reason(static_cast< TransactionItemReason >(query.get< int >("reason")));
    ti.set_repoid(query.get< std::string >("repoid"));
    ti.set_state(static_cast< TransactionItemState >(query.get< int >("state")));
    item->setId(query.get< int >("item_id"));
    item->set_name(query.get< std::string >("name"));
    item->set_epoch(query.get< int >("epoch"));
    item->set_version(query.get< std::string >("version"));
    item->set_release(query.get< std::string >("release"));
    item->set_arch(query.get< std::string >("arch"));
    return ti.get_id();
}


static const char * SQL_RPM_INSERT = R"**(
    INSERT INTO
        rpm (
            item_id,
            name,
            epoch,
            version,
            release,
            arch
        )
    VALUES
        (?, ?, ?, ?, ?, ?)
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> rpm_insert_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_RPM_INSERT);
    return query;
}


int64_t rpm_insert(libdnf::utils::SQLite3::Statement & query, const Package & rpm) {
    query.bindv(
        rpm.getId(),
        rpm.get_name(),
        rpm.get_epoch(),
        rpm.get_version(),
        rpm.get_release(),
        rpm.get_arch()
    );
    query.step();
    int64_t result = query.last_insert_rowid();
    query.reset();
    return result;
}


static const char * SQL_RPM_SELECT_PK = R"**(
    SELECT
        item_id
    FROM
        rpm
    WHERE
        name=?
        AND epoch=?
        AND version=?
        AND release=?
        AND arch=?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Statement> rpm_select_pk_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_RPM_SELECT_PK);
    return query;
}


int64_t rpm_select_pk(libdnf::utils::SQLite3::Statement & query, const Package & rpm) {
    query.bindv(
        rpm.get_name(),
        rpm.get_epoch(),
        rpm.get_version(),
        rpm.get_release(),
        rpm.get_arch()
    );

    int64_t result = 0;
    if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        result = query.get<int64_t>(0);
    }
    query.reset();
    return result;
}


static const char * SQL_RPM_SELECT = R"**(
    SELECT
        item_id,
        name,
        epoch,
        version,
        release,
        arch
    FROM
        rpm
    WHERE
        item_id = ?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Query> rpm_select_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_RPM_SELECT);
    return query;
}


bool rpm_select(libdnf::utils::SQLite3::Query & query, int64_t rpm_id, Package & rpm) {
    bool result = false;
    query.bindv(rpm_id);

    if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        rpm.setId(query.get<int>("item_id"));
        rpm.set_name(query.get<std::string>("name"));
        rpm.set_epoch(query.get<uint32_t>("epoch"));
        rpm.set_version(query.get<std::string>("version"));
        rpm.set_release(query.get<std::string>("release"));
        rpm.set_arch(query.get<std::string>("arch"));
        result = true;
    }

    query.reset();
    return result;
}


std::vector<std::shared_ptr<TransactionItem>> get_transaction_packages(Transaction & trans) {
    std::vector<std::shared_ptr<TransactionItem>> result;

    auto query = rpm_transaction_item_select_new_query(trans.get_connection(), trans.get_id());
    while (query->step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto trans_item = std::make_shared< TransactionItem >(trans);
        rpm_transaction_item_select(*query, *trans_item);
        result.push_back(trans_item);
    }

    return result;
}


}  // namespace libdnf::transaction
