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


namespace libdnf::transaction {


/*
CompsGroup::CompsGroup(libdnf::utils::SQLite3Ptr conn)
  : Item{conn}
  , packageTypes(CompsPackageType::DEFAULT)
{
}
*/


CompsGroup::CompsGroup(Transaction & trans, int64_t pk)
  : Item{trans}
{
    dbSelect(pk);
}


void
CompsGroup::save()
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
    query.step();

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


/**
 * Lazy loader for packages associated with the group.
 * \return list of packages associated with the group (installs and excludes).
 */
std::vector< CompsGroupPackagePtr >
CompsGroup::getPackages()
{
    if (packages.empty()) {
        loadPackages();
    }
    return packages;
}

void
CompsGroup::loadPackages()
{
    const char *sql =
        "SELECT "
        "  * "
        "FROM "
        "  comps_group_package "
        "WHERE "
        "  group_id = ?";
    libdnf::utils::SQLite3::Query query(trans.get_connection(), sql);
    query.bindv(getId());

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto pkg = std::make_shared< CompsGroupPackage >(*this);
        pkg->set_id(query.get< int >("id"));
        pkg->set_name(query.get< std::string >("name"));
        pkg->set_installed(query.get< bool >("installed"));
        pkg->set_package_type(static_cast< CompsPackageType >(query.get< int >("pkg_type")));
        packages.push_back(pkg);
    }
}

CompsGroupPackagePtr
CompsGroup::add_package(std::string name, bool installed, CompsPackageType pkg_type)
{
    // try to find an existing package and override it with the new values
    CompsGroupPackagePtr pkg = nullptr;
    for (auto & i : packages) {
        if (i->get_name() == name) {
            pkg = i;
            break;
        }
    }

    if (pkg == nullptr) {
        pkg = std::make_shared< CompsGroupPackage >(*this);
        packages.push_back(pkg);
    }

    pkg->set_name(name);
    pkg->set_installed(installed);
    pkg->set_package_type(pkg_type);
    return pkg;
}

CompsGroupPackage::CompsGroupPackage(CompsGroup & group)
  : group(group)
{
}

void
CompsGroupPackage::save()
{
    if (get_id() == 0) {
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
    libdnf::utils::SQLite3::Statement query(get_group().trans.get_connection(), sql);
    query.bindv(
        get_group().getId(), get_name(), get_installed(), static_cast< int >(get_package_type()));
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
    libdnf::utils::SQLite3::Statement query(get_group().trans.get_connection(), sql);
    query.bindv(
        get_name(), get_installed(), static_cast< int >(get_package_type()), get_id());
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

    libdnf::utils::SQLite3::Statement query(get_group().trans.get_connection(), sql);
    query.bindv(get_name(), get_group().getId());
    libdnf::utils::SQLite3::Statement::StepResult result = query.step();

    if (result == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        set_id(query.get< int >(0));
        dbUpdate();
    } else {
        // insert and get the ID back
        dbInsert();
    }
}


}  // namespace libdnf::transaction
