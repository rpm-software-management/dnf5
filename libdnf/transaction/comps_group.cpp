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


#include <algorithm>

#include "comps_group.hpp"
#include "transaction.hpp"
#include "transaction_item.hpp"

#include "libdnf/transaction/db/comps_group_package.hpp"


namespace libdnf::transaction {


CompsGroup::CompsGroup(Transaction & trans, int64_t pk) : Item{trans} {
    dbSelect(pk);
    comps_group_packages_select(*this);
}


void CompsGroup::save() {
    dbInsert();
    comps_group_packages_insert(*this);
}


void
CompsGroup::dbSelect(int64_t pk)
{
    const char *sql =
        "SELECT "
        "  groupid, "
        "  name, "
        "  translated_name, "
        "  pkg_types "
        "FROM "
        "  comps_group "
        "WHERE "
        "  item_id = ?";
    libdnf::utils::SQLite3::Query query(trans.get_connection(), sql);
    query.bindv(pk);

    if (query.step() != libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        /// TODO(dmach): replace with a different exception type
        throw std::runtime_error("Could not find a record in table 'comps_group' with item_id == " + pk);
    }
    setId(pk);
    set_group_id(query.get< std::string >("groupid"));
    set_name(query.get< std::string >("name"));
    set_translated_name(query.get< std::string >("translated_name"));
    set_package_types(static_cast< CompsPackageType >(query.get< int >("pkg_types")));
}

void
CompsGroup::dbInsert()
{
    // populates this->id
    Item::save();

    const char *sql =
        "INSERT INTO "
        "  comps_group ( "
        "    item_id, "
        "    groupid, "
        "    name, "
        "    translated_name, "
        "    pkg_types "
        "  ) "
        "VALUES "
        "  (?, ?, ?, ?, ?)";
    libdnf::utils::SQLite3::Statement query(trans.get_connection(), sql);
    query.bindv(getId(),
                get_group_id(),
                get_name(),
                get_translated_name(),
                static_cast< int >(get_package_types()));
    query.step();
}

static TransactionItemPtr
compsGroupTransactionItemFromQuery(Transaction & trans, libdnf::utils::SQLite3::Query & query)
{
    auto trans_item = std::make_shared< TransactionItem >(trans);
    auto item = std::make_shared< CompsGroup >(trans);
    trans_item->setItem(item);

    trans_item->set_id(query.get< int >("ti_id"));
    trans_item->set_action(static_cast< TransactionItemAction >(query.get< int >("ti_action")));
    trans_item->set_reason(static_cast< TransactionItemReason >(query.get< int >("ti_reason")));
    trans_item->set_state(static_cast< TransactionItemState >(query.get< int >("ti_state")));
    item->setId(query.get< int >("item_id"));
    item->set_group_id(query.get< std::string >("groupid"));
    item->set_name(query.get< std::string >("name"));
    item->set_translated_name(query.get< std::string >("translated_name"));
    item->set_package_types(static_cast< CompsPackageType >(query.get< int >("pkg_types")));
    comps_group_packages_select(*item);

    return trans_item;
}

/*
TransactionItemPtr
CompsGroup::getTransactionItem(libdnf::utils::SQLite3Ptr conn, const std::string &groupid)
{
    const char *sql = R"**(
        SELECT
            ti.trans_id,
            ti.id as ti_id,
            ti.state as ti_state,
            ti.action as ti_action,
            ti.reason as ti_reason,
            i.item_id,
            i.groupid,
            i.name,
            i.translated_name,
            i.pkg_types
        FROM
            trans_item ti
        JOIN
            comps_group i USING (item_id)
        JOIN
            trans t ON ti.trans_id = t.id
        WHERE
            t.state = 1
            AND ti.action not in (3, 5, 7)
            AND i.groupid = ?
        ORDER BY
            ti.trans_id DESC
    )**";

    libdnf::utils::SQLite3::Query query(*conn, sql);
    query.bindv(groupid);
    if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto trans_item =
            compsGroupTransactionItemFromQuery(conn, query, query.get< int64_t >("trans_id"));
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
CompsGroup::getTransactionItemsByPattern(libdnf::utils::SQLite3Ptr conn, const std::string &pattern)
{
    const char *sql = R"**(
        SELECT DISTINCT
            groupid
        FROM
            comps_group
        WHERE
            groupid LIKE ?
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
        auto groupid = query.get< std::string >("groupid");
        auto trans_item = getTransactionItem(conn, groupid);
        if (!trans_item) {
            continue;
        }
        result.push_back(trans_item);
    }
    return result;
}
*/

std::vector< TransactionItemPtr >
CompsGroup::getTransactionItems(Transaction & trans)
{
    std::vector< TransactionItemPtr > result;

    const char *sql = R"**(
        SELECT
            ti.id as ti_id,
            ti.action as ti_action,
            ti.reason as ti_reason,
            ti.state as ti_state,
            i.item_id,
            i.groupid,
            i.name,
            i.translated_name,
            i.pkg_types
        FROM
            trans_item ti
        JOIN
            comps_group i USING (item_id)
        WHERE
            ti.trans_id = ?
    )**";
    libdnf::utils::SQLite3::Query query(trans.get_connection(), sql);
    query.bindv(trans.get_id());

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto trans_item = compsGroupTransactionItemFromQuery(trans, query);
        result.push_back(trans_item);
    }
    return result;
}


CompsGroupPackage & CompsGroup::new_package() {
    auto pkg = new CompsGroupPackage(*this);
    auto pkg_ptr = std::unique_ptr<CompsGroupPackage>(std::move(pkg));
    // TODO(dmach): following lines are not thread-safe
    packages.push_back(std::move(pkg_ptr));
    return *packages.back();
}


CompsGroupPackage::CompsGroupPackage(CompsGroup & group)
  : group(group)
{
}


}  // namespace libdnf::transaction
