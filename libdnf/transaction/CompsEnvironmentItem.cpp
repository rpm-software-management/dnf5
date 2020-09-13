/*
 * Copyright (C) 2017-2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <algorithm>

#include "CompsEnvironmentItem.hpp"

namespace libdnf::transaction {

typedef const char *string;

CompsEnvironmentItem::CompsEnvironmentItem(libdnf::utils::SQLite3Ptr conn)
  : Item{conn}
  , packageTypes{CompsPackageType::DEFAULT}
{
}

CompsEnvironmentItem::CompsEnvironmentItem(libdnf::utils::SQLite3Ptr conn, int64_t pk)
  : Item{conn}
  , packageTypes{CompsPackageType::DEFAULT}
{
    dbSelect(pk);
}

void
CompsEnvironmentItem::save()
{
    if (getId() == 0) {
        dbInsert();
    } else {
        // dbUpdate();
    }
    for (const auto &i : getGroups()) {
        i->save();
    }
}

void
CompsEnvironmentItem::dbSelect(int64_t pk)
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
    libdnf::utils::SQLite3::Query query(*conn.get(), sql);
    query.bindv(pk);
    query.step();

    setId(pk);
    setEnvironmentId(query.get< std::string >("environmentid"));
    setName(query.get< std::string >("name"));
    setTranslatedName(query.get< std::string >("translated_name"));
    setPackageTypes(static_cast< CompsPackageType >(query.get< int >("pkg_types")));
}

void
CompsEnvironmentItem::dbInsert()
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
    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(getId(),
                getEnvironmentId(),
                getName(),
                getTranslatedName(),
                static_cast< int >(getPackageTypes()));
    query.step();
}

static TransactionItemPtr
compsEnvironmentTransactionItemFromQuery(libdnf::utils::SQLite3Ptr conn, libdnf::utils::SQLite3::Query &query, int64_t transID)
{
    auto trans_item = std::make_shared< TransactionItem >(conn, transID);
    auto item = std::make_shared< CompsEnvironmentItem >(conn);

    trans_item->setItem(item);
    trans_item->set_id(query.get< int >("ti_id"));
    trans_item->set_action(static_cast< TransactionItemAction >(query.get< int >("ti_action")));
    trans_item->set_reason(static_cast< TransactionItemReason >(query.get< int >("ti_reason")));
    trans_item->set_state(static_cast< TransactionItemState >(query.get< int >("ti_state")));
    item->setId(query.get< int >("item_id"));
    item->setEnvironmentId(query.get< std::string >("environmentid"));
    item->setName(query.get< std::string >("name"));
    item->setTranslatedName(query.get< std::string >("translated_name"));
    item->setPackageTypes(static_cast< CompsPackageType >(query.get< int >("pkg_types")));

    return trans_item;
}

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
            /* see comment in TransactionItem.hpp - TransactionItemAction */
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

std::vector< TransactionItemPtr >
CompsEnvironmentItem::getTransactionItems(libdnf::utils::SQLite3Ptr conn, int64_t transactionId)
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
    libdnf::utils::SQLite3::Query query(*conn.get(), sql);
    query.bindv(transactionId);

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto trans_item = std::make_shared< TransactionItem >(conn, transactionId);
        auto item = std::make_shared< CompsEnvironmentItem >(conn);
        trans_item->setItem(item);

        trans_item->set_id(query.get< int >(0));
        trans_item->set_state(static_cast< TransactionItemState >(query.get< int >(1)));
        item->setId(query.get< int >(2));
        item->setEnvironmentId(query.get< std::string >(3));
        item->setName(query.get< std::string >(4));
        item->setTranslatedName(query.get< std::string >(5));
        item->setPackageTypes(static_cast< CompsPackageType >(query.get< int >(6)));

        result.push_back(trans_item);
    }
    return result;
}

std::string
CompsEnvironmentItem::toStr() const
{
    return "@" + getEnvironmentId();
}

/**
 * Lazy loader for groups associated with the environment.
 * \return vector of groups associated with the environment
 */
std::vector< CompsEnvironmentGroupPtr >
CompsEnvironmentItem::getGroups()
{
    if (groups.empty()) {
        loadGroups();
    }
    return groups;
}

void
CompsEnvironmentItem::loadGroups()
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
    libdnf::utils::SQLite3::Query query(*conn.get(), sql);
    query.bindv(getId());

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto group = std::make_shared< CompsEnvironmentGroup >(*this);
        group->setId(query.get< int >("id"));
        group->setGroupId(query.get< std::string >("groupid"));

        group->setInstalled(query.get< bool >("installed"));
        group->setGroupType(static_cast< CompsPackageType >(query.get< int >("group_type")));
        groups.push_back(group);
    }
}

CompsEnvironmentGroupPtr
CompsEnvironmentItem::addGroup(std::string groupId, bool installed, CompsPackageType groupType)
{
    // try to find an existing group and override it with the new values
    CompsEnvironmentGroupPtr grp = nullptr;
    for (auto & i : groups) {
        if (i->getGroupId() == groupId) {
            grp = i;
            break;
        }
    }

    if (grp == nullptr) {
        grp = std::make_shared< CompsEnvironmentGroup >(*this);
        groups.push_back(grp);
    }

    grp->setGroupId(groupId);
    grp->setInstalled(installed);
    grp->setGroupType(groupType);
    return grp;
}

CompsEnvironmentGroup::CompsEnvironmentGroup(CompsEnvironmentItem &environment)
  : environment(environment)
{
}

void
CompsEnvironmentGroup::save()
{
    if (getId() == 0) {
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
    libdnf::utils::SQLite3::Statement query(*getEnvironment().conn, sql);
    query.bindv(
        getEnvironment().getId(), getGroupId(), getInstalled(), static_cast< int >(getGroupType()));
    query.step();
    setId(getEnvironment().conn->last_insert_rowid());
}

}  // namespace libdnf::transaction
