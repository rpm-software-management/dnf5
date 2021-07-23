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


// TODO(dmach): rename the table to trans_runtime_packages or trans_runtime_rpms?


#include "libdnf/transaction/db/trans_with.hpp"

#include "libdnf/rpm/nevra.hpp"
#include "libdnf/transaction/db/item.hpp"
#include "libdnf/transaction/db/rpm.hpp"
#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <memory>


namespace libdnf::transaction {


static const char * SQL_TRANS_WITH_SELECT = R"**(
    SELECT
        item_id,
        name,
        epoch,
        version,
        release,
        arch
    FROM
        trans_with
    JOIN
        rpm USING (item_id)
    WHERE
        trans_id = ?
)**";


std::unique_ptr<libdnf::utils::SQLite3::Query> trans_with_select_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_TRANS_WITH_SELECT);
    return query;
}


std::set<std::string> load_transaction_runtime_packages(libdnf::utils::SQLite3 & conn, Transaction & trans) {
    std::set<std::string> result;

    auto query = trans_with_select_new_query(conn);
    query->bindv(trans.get_id());
    while (query->step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        libdnf::rpm::Nevra nevra;
        nevra.set_name(query->get<std::string>("name"));
        nevra.set_epoch(query->get<std::string>("epoch"));
        nevra.set_version(query->get<std::string>("version"));
        nevra.set_release(query->get<std::string>("release"));
        nevra.set_arch(query->get<std::string>("arch"));
        result.insert(libdnf::rpm::to_nevra_string(nevra));
    }

    return result;
}


const char * SQL_TRANS_WITH_INSERT = R"**(
    INSERT OR REPLACE INTO
        trans_with (
            trans_id,
            item_id
        )
    VALUES
        (?, ?)
)**";


std::unique_ptr<libdnf::utils::SQLite3::Query> trans_with_insert_new_query(libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_TRANS_WITH_INSERT);
    return query;
}


void save_transaction_runtime_packages(libdnf::utils::SQLite3 & conn, Transaction & trans) {
    if (trans.get_runtime_packages().empty()) {
        return;
    }

    auto query_rpm_select_pk = rpm_select_pk_new_query(conn);
    auto query_rpm_insert = rpm_insert_new_query(conn);
    auto query_item_insert = item_insert_new_query(conn, TransactionItemType::RPM);
    auto query_trans_with_insert = trans_with_insert_new_query(conn);


    for (auto & ti : trans.get_runtime_packages()) {
        auto nevras = rpm::Nevra::parse(ti, {libdnf::rpm::Nevra::Form::NEVRA});
        // TODO(jmracek) What about to create a constructor of Package that will accept NEVRA string
        Package rpm(trans);
        libdnf::rpm::copy_nevra_attributes(*nevras.begin(), rpm);
        auto pk = rpm_select_pk(*query_rpm_select_pk, rpm);
        if (pk == 0) {
            pk = item_insert(*query_item_insert);
            rpm.set_item_id(pk);
            rpm_insert(*query_rpm_insert, rpm);
        }
        query_trans_with_insert->bindv(trans.get_id(), pk);
        query_trans_with_insert->step();
        query_trans_with_insert->reset();
    }
}


}  // namespace libdnf::transaction
