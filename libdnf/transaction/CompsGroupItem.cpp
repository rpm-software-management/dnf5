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

#include "CompsGroupItem.hpp"
#include "TransactionItem.hpp"

namespace libdnf {

CompsGroupItem::CompsGroupItem(SQLite3Ptr conn)
  : Item{conn}
  , packageTypes(CompsPackageType::DEFAULT)
{
}

CompsGroupItem::CompsGroupItem(SQLite3Ptr conn, int64_t pk)
  : Item{conn}
  , packageTypes(CompsPackageType::DEFAULT)
{
    dbSelect(pk);
}

void
CompsGroupItem::save()
{
    if (getId() == 0) {
        dbInsert();
    } else {
        // dbUpdate();
    }
    for (auto i : getPackages()) {
        i->save();
    }
}

void
CompsGroupItem::dbSelect(int64_t pk)
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
    SQLite3::Query query(*conn.get(), sql);
    query.bindv(pk);
    query.step();

    setId(pk);
    setGroupId(query.get< std::string >("groupid"));
    setName(query.get< std::string >("name"));
    setTranslatedName(query.get< std::string >("translated_name"));
    setPackageTypes(static_cast< CompsPackageType >(query.get< int >("pkg_types")));
}

void
CompsGroupItem::dbInsert()
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
    SQLite3::Statement query(*conn.get(), sql);
    query.bindv(getId(),
                getGroupId(),
                getName(),
                getTranslatedName(),
                static_cast< int >(getPackageTypes()));
    query.step();
}

static TransactionItemPtr
compsGroupTransactionItemFromQuery(SQLite3Ptr conn, SQLite3::Query &query, int64_t transID)
{

    auto trans_item = std::make_shared< TransactionItem >(conn, transID);
    auto item = std::make_shared< CompsGroupItem >(conn);
    trans_item->setItem(item);

    trans_item->setId(query.get< int >("ti_id"));
    trans_item->setAction(static_cast< TransactionItemAction >(query.get< int >("ti_action")));
    trans_item->setReason(static_cast< TransactionItemReason >(query.get< int >("ti_reason")));
    trans_item->setState(static_cast< TransactionItemState >(query.get< int >("ti_state")));
    item->setId(query.get< int >("item_id"));
    item->setGroupId(query.get< std::string >("groupid"));
    item->setName(query.get< std::string >("name"));
    item->setTranslatedName(query.get< std::string >("translated_name"));
    item->setPackageTypes(static_cast< CompsPackageType >(query.get< int >("pkg_types")));

    return trans_item;
}

TransactionItemPtr
CompsGroupItem::getTransactionItem(SQLite3Ptr conn, const std::string &groupid)
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
            /* see comment in TransactionItem.hpp - TransactionItemAction */
            AND ti.action not in (3, 5, 7)
            AND i.groupid = ?
        ORDER BY
            ti.trans_id DESC
    )**";

    SQLite3::Query query(*conn, sql);
    query.bindv(groupid);
    if (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto trans_item =
            compsGroupTransactionItemFromQuery(conn, query, query.get< int64_t >("trans_id"));
        if (trans_item->getAction() == TransactionItemAction::REMOVE) {
            return nullptr;
        }
        return trans_item;
    }
    return nullptr;
}

std::vector< TransactionItemPtr >
CompsGroupItem::getTransactionItemsByPattern(SQLite3Ptr conn, const std::string &pattern)
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
    SQLite3 privateConn(conn->getPath());
    SQLite3::Query query(privateConn, sql);
    std::string pattern_sql = pattern;
    std::replace(pattern_sql.begin(), pattern_sql.end(), '*', '%');
    query.bindv(pattern, pattern, pattern);
    while (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto groupid = query.get< std::string >("groupid");
        auto trans_item = getTransactionItem(conn, groupid);
        if (!trans_item) {
            continue;
        }
        result.push_back(trans_item);
    }
    return result;
}

std::vector< TransactionItemPtr >
CompsGroupItem::getTransactionItems(SQLite3Ptr conn, int64_t transactionId)
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
    SQLite3::Query query(*conn.get(), sql);
    query.bindv(transactionId);

    while (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto trans_item = compsGroupTransactionItemFromQuery(conn, query, transactionId);
        result.push_back(trans_item);
    }
    return result;
}

std::string
CompsGroupItem::toStr() const
{
    return "@" + getGroupId();
}

/**
 * Lazy loader for packages associated with the group.
 * \return list of packages associated with the group (installs and excludes).
 */
std::vector< CompsGroupPackagePtr >
CompsGroupItem::getPackages()
{
    if (packages.empty()) {
        loadPackages();
    }
    return packages;
}

void
CompsGroupItem::loadPackages()
{
    const char *sql =
        "SELECT "
        "  * "
        "FROM "
        "  comps_group_package "
        "WHERE "
        "  group_id = ?";
    SQLite3::Query query(*conn.get(), sql);
    query.bindv(getId());

    while (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto pkg = std::make_shared< CompsGroupPackage >(*this);
        pkg->setId(query.get< int >("id"));
        pkg->setName(query.get< std::string >("name"));
        pkg->setInstalled(query.get< bool >("installed"));
        pkg->setPackageType(static_cast< CompsPackageType >(query.get< int >("pkg_type")));
        packages.push_back(pkg);
    }
}

CompsGroupPackagePtr
CompsGroupItem::addPackage(std::string name, bool installed, CompsPackageType pkgType)
{
    // try to find an existing package and override it with the new values
    CompsGroupPackagePtr pkg = nullptr;
    for (auto & i : packages) {
        if (i->getName() == name) {
            pkg = i;
            break;
        }
    }

    if (pkg == nullptr) {
        pkg = std::make_shared< CompsGroupPackage >(*this);
        packages.push_back(pkg);
    }

    pkg->setName(name);
    pkg->setInstalled(installed);
    pkg->setPackageType(pkgType);
    return pkg;
}

CompsGroupPackage::CompsGroupPackage(CompsGroupItem &group)
  : group(group)
{
}

void
CompsGroupPackage::save()
{
    if (getId() == 0) {
        dbSelectOrInsert();
    } else {
        dbUpdate();
    }
}

void
CompsGroupPackage::dbInsert()
{
    const char *sql = R"**(
        INSERT INTO
            comps_group_package (
                group_id,
                name,
                installed,
                pkg_type
            )
        VALUES
            (?, ?, ?, ?)
    )**";
    SQLite3::Statement query(*getGroup().conn.get(), sql);
    query.bindv(
        getGroup().getId(), getName(), getInstalled(), static_cast< int >(getPackageType()));
    query.step();
}

void
CompsGroupPackage::dbUpdate()
{
    const char *sql = R"**(
        UPDATE
            comps_group_package
        SET
            name=?,
            installed=?,
            pkg_type=?
        WHERE
            id = ?
    )**";
    SQLite3::Statement query(*getGroup().conn.get(), sql);
    query.bindv(
        getName(), getInstalled(), static_cast< int >(getPackageType()), getId());
    query.step();
}


void
CompsGroupPackage::dbSelectOrInsert()
{
    const char *sql = R"**(
        SELECT
            id
        FROM
          comps_group_package
        WHERE
            name = ?
            AND group_id = ?
    )**";

    SQLite3::Statement query(*getGroup().conn.get(), sql);
    query.bindv(getName(), getGroup().getId());
    SQLite3::Statement::StepResult result = query.step();

    if (result == SQLite3::Statement::StepResult::ROW) {
        setId(query.get< int >(0));
        dbUpdate();
    } else {
        // insert and get the ID back
        dbInsert();
    }
}

} // namespace libdnf
