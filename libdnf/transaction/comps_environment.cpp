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

#include "comps_environment.hpp"
#include "transaction.hpp"


namespace libdnf::transaction {


typedef const char *string;


CompsEnvironment::CompsEnvironment(Transaction & trans, int64_t pk)
  : Item{trans}
{
    dbSelect(pk);
}


void
CompsEnvironment::save()
{
    if (getId() == 0) {
        dbInsert();
    } else {
        // dbUpdate();
    }
    for (const auto &i : get_groups()) {
        i->save();
    }
}

void
CompsEnvironment::dbSelect(int64_t pk)
{
    const char *sql = R"**(
        SELECT
            environmentid,
            name,
            translated_name,
            pkg_types
        FROM
            comps_environment
        WHERE
            item_id = ?
    )**";
    libdnf::utils::SQLite3::Query query(trans.get_connection(), sql);
    query.bindv(pk);
    query.step();

    setId(pk);
    set_environment_id(query.get< std::string >("environmentid"));
    set_name(query.get< std::string >("name"));
    set_translated_name(query.get< std::string >("translated_name"));
    set_package_types(static_cast< CompsPackageType >(query.get< int >("pkg_types")));
}

void
CompsEnvironment::dbInsert()
{
    // populates this->id
    Item::save();

    const char *sql = R"**(
        INSERT INTO
            comps_environment (
                item_id,
                environmentid,
                name,
                translated_name,
                pkg_types
            )
        VALUES
            (?, ?, ?, ?, ?)
    )**";
    libdnf::utils::SQLite3::Statement query(trans.get_connection(), sql);
    query.bindv(getId(),
                get_environment_id(),
                get_name(),
                get_translated_name(),
                static_cast< int >(get_package_types()));
    query.step();
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

std::vector< TransactionItemPtr >
CompsEnvironment::getTransactionItems(Transaction & trans)
{
    std::vector< TransactionItemPtr > result;

    const char *sql = R"**(
        SELECT
            ti.id,
            ti.state,
            i.item_id,
            i.environmentid,
            i.name,
            i.translated_name,
            i.pkg_types
        FROM
            trans_item ti,
            comps_environment i
        WHERE
            ti.trans_id = ?
            AND ti.item_id = i.item_id
    )**";
    libdnf::utils::SQLite3::Query query(trans.get_connection(), sql);
    query.bindv(trans.get_id());

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto trans_item = std::make_shared< TransactionItem >(trans);
        auto item = std::make_shared< CompsEnvironment >(trans);
        trans_item->setItem(item);

        trans_item->set_id(query.get< int >(0));
        trans_item->set_state(static_cast< TransactionItemState >(query.get< int >(1)));
        item->setId(query.get< int >(2));
        item->set_environment_id(query.get< std::string >(3));
        item->set_name(query.get< std::string >(4));
        item->set_translated_name(query.get< std::string >(5));
        item->set_package_types(static_cast< CompsPackageType >(query.get< int >(6)));

        result.push_back(trans_item);
    }
    return result;
}

std::string
CompsEnvironment::toStr() const
{
    return "@" + get_environment_id();
}

/**
 * Lazy loader for groups associated with the environment.
 * \return vector of groups associated with the environment
 */
std::vector< CompsEnvironmentGroupPtr >
CompsEnvironment::get_groups()
{
    if (groups.empty()) {
        loadGroups();
    }
    return groups;
}

void
CompsEnvironment::loadGroups()
{
    const char *sql = R"**(
        SELECT
            *
        FROM
            comps_environment_group
        WHERE
            environment_id = ?
        ORDER BY
            groupid ASC
    )**";
    libdnf::utils::SQLite3::Query query(trans.get_connection(), sql);
    query.bindv(getId());

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto group = std::make_shared< CompsEnvironmentGroup >(*this);
        group->set_id(query.get< int >("id"));
        group->set_group_id(query.get< std::string >("groupid"));

        group->set_installed(query.get< bool >("installed"));
        group->set_group_type(static_cast< CompsPackageType >(query.get< int >("group_type")));
        groups.push_back(group);
    }
}

CompsEnvironmentGroupPtr
CompsEnvironment::add_group(std::string group_id, bool installed, CompsPackageType group_type)
{
    // try to find an existing group and override it with the new values
    CompsEnvironmentGroupPtr grp = nullptr;
    for (auto & i : groups) {
        if (i->get_group_id() == group_id) {
            grp = i;
            break;
        }
    }

    if (grp == nullptr) {
        grp = std::make_shared< CompsEnvironmentGroup >(*this);
        groups.push_back(grp);
    }

    grp->set_group_id(group_id);
    grp->set_installed(installed);
    grp->set_group_type(group_type);
    return grp;
}

CompsEnvironmentGroup::CompsEnvironmentGroup(CompsEnvironment &environment)
  : environment(environment)
{
}

void
CompsEnvironmentGroup::save()
{
    if (get_id() == 0) {
        dbInsert();
    } else {
        // dbUpdate();
    }
}

void
CompsEnvironmentGroup::dbInsert()
{
    const char *sql = R"**(
        INSERT INTO
            comps_environment_group (
                environment_id,
                groupid,
                installed,
                group_type
            )
        VALUES
            (?, ?, ?, ?)
    )**";
    libdnf::utils::SQLite3::Statement query(get_environment().trans.get_connection(), sql);
    query.bindv(
        get_environment().getId(), get_group_id(), get_installed(), static_cast< int >(get_group_type()));
    query.step();
    set_id(query.last_insert_rowid());
}

}  // namespace libdnf::transaction
