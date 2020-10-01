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


#include "comps_environment.hpp"
#include "transaction.hpp"

#include "libdnf/transaction/db/comps_environment.hpp"
#include "libdnf/transaction/db/comps_environment_group.hpp"


namespace libdnf::transaction {


void CompsEnvironment::save() {
    auto query = comps_environment_insert_new_query(trans.get_connection());
    comps_environment_insert(*query, *this);
    comps_environment_groups_insert(*this);
}


/*
static TransactionItemPtr
compsEnvironmentTransactionItemFromQuery(libdnf::utils::SQLite3Ptr conn, libdnf::utils::SQLite3::Query &query, Transaction & trans)
{
    auto trans_item = std::make_shared< TransactionItem >(trans);
    auto item = std::make_shared< CompsEnvironmentItem >(conn);

    trans_item->setItem(item);
    trans_item->set_id(query.get< int >("ti_id"));
    trans_item->set_action(static_cast< TransactionItemAction >(query.get< int >("ti_action")));
    trans_item->set_reason(static_cast< TransactionItemReason >(query.get< int >("ti_reason")));
    trans_item->set_state(static_cast< TransactionItemState >(query.get< int >("ti_state")));
    item->setId(query.get< int >("item_id"));
    item->set_environment_id(query.get< std::string >("environmentid"));
    item->setName(query.get< std::string >("name"));
    item->set_translated_name(query.get< std::string >("translated_name"));
    item->set_package_types(static_cast< CompsPackageType >(query.get< int >("pkg_types")));

    return trans_item;
}
*/

/*
TransactionItemPtr
CompsEnvironmentItem::getTransactionItem(libdnf::utils::SQLite3Ptr conn, const std::string &envid)
{
    const char *sql = R"**(
        SELECT
            ti.trans_id,
            ti.id as ti_id,
            ti.state as ti_state,
            ti.action as ti_action,
            ti.reason as ti_reason,
            i.item_id,
            i.environmentid,
            i.name,
            i.translated_name,
            i.pkg_types
        FROM
            trans_item ti
        JOIN
            comps_environment i USING (item_id)
        JOIN
            trans t ON ti.trans_id = t.id
        WHERE
            t.state = 1
            AND ti.action not in (3, 5, 7)
            AND i.environmentid = ?
        ORDER BY
            ti.trans_id DESC
        LIMIT 1
    )**";

    libdnf::utils::SQLite3::Query query(*conn, sql);
    query.bindv(envid);
    if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto trans_item =
            compsEnvironmentTransactionItemFromQuery(conn, query, query.get< int64_t >("trans_id"));

        if (trans_item->get_action() == TransactionItemAction::REMOVE) {
            return nullptr;
        }
        return trans_item;
    }
    return nullptr;
}
*/

/*
std::vector< TransactionItemPtr >
CompsEnvironmentItem::getTransactionItemsByPattern(libdnf::utils::SQLite3Ptr conn, const std::string &pattern)
{
    string sql = R"**(
            SELECT DISTINCT
                environmentid
            FROM
                comps_environment
            WHERE
                environmentid LIKE ?
                OR name LIKE ?
                OR translated_name LIKE ?
        )**";

    std::vector< TransactionItemPtr > result;

    // HACK: create a private connection to avoid undefined behavior
    // after forking process in Anaconda
    libdnf::utils::SQLite3 privateConn(conn->get_path());
    libdnf::utils::SQLite3::Query query(privateConn, sql);
    std::string pattern_sql = pattern;
    std::replace(pattern_sql.begin(), pattern_sql.end(), '*', '%');
    query.bindv(pattern, pattern, pattern);
    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto groupid = query.get< std::string >("environmentid");
        auto trans_item = getTransactionItem(conn, groupid);
        if (!trans_item) {
            continue;
        }
        result.push_back(trans_item);
    }
    return result;
}
*/


std::string
CompsEnvironment::toStr() const
{
    return "@" + get_environment_id();
}


CompsEnvironmentGroup::CompsEnvironmentGroup(CompsEnvironment &environment)
  : environment(environment)
{
}


CompsEnvironmentGroup & CompsEnvironment::new_group() {
    auto grp = new CompsEnvironmentGroup(*this);
    auto grp_ptr = std::unique_ptr<CompsEnvironmentGroup>(grp);
    // TODO(dmach): following lines are not thread-safe
    groups.push_back(std::move(grp_ptr));
    return *groups.back();
}


}  // namespace libdnf::transaction
