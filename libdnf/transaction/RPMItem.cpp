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
#include <map>
#include <sstream>

//#include "../hy-subject.h"
#include "libdnf/rpm/nevra.hpp"

#include "RPMItem.hpp"

namespace libdnf::transaction {

RPMItem::RPMItem(libdnf::utils::SQLite3Ptr conn)
  : Item{conn}
{
}

RPMItem::RPMItem(libdnf::utils::SQLite3Ptr conn, int64_t pk)
  : Item{conn}
{
    dbSelect(pk);
}

void
RPMItem::save()
{
    if (getId() == 0) {
        dbSelectOrInsert();
    } else {
        // TODO: dbUpdate() ?
    }
}

void
RPMItem::dbSelect(int64_t pk)
{
    const char *sql =
        "SELECT "
        "  name, "
        "  epoch, "
        "  version, "
        "  release, "
        "  arch "
        "FROM "
        "  rpm "
        "WHERE "
        "  item_id = ?";
    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(pk);
    query.step();

    setId(pk);
    setName(query.get< std::string >(0));
    setEpoch(query.get< int >(1));
    setVersion(query.get< std::string >(2));
    setRelease(query.get< std::string >(3));
    setArch(query.get< std::string >(4));
}

void
RPMItem::dbInsert()
{
    // populates this->id
    Item::save();

    const char *sql =
        "INSERT INTO "
        "  rpm "
        "VALUES "
        "  (?, ?, ?, ?, ?, ?)";
    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(getId(), getName(), getEpoch(), getVersion(), getRelease(), getArch());
    query.step();
}

static TransactionItemPtr
transactionItemFromQuery(libdnf::utils::SQLite3Ptr conn, libdnf::utils::SQLite3::Query &query, int64_t transID)
{
    auto trans_item = std::make_shared< TransactionItem >(conn, transID);
    auto item = std::make_shared< RPMItem >(conn);
    trans_item->setItem(item);
    trans_item->setId(query.get< int >("id"));
    trans_item->setAction(static_cast< TransactionItemAction >(query.get< int >("action")));
    trans_item->setReason(static_cast< TransactionItemReason >(query.get< int >("reason")));
    trans_item->setRepoid(query.get< std::string >("repoid"));
    trans_item->setState(static_cast< TransactionItemState >(query.get< int >("state")));
    item->setId(query.get< int >("item_id"));
    item->setName(query.get< std::string >("name"));
    item->setEpoch(query.get< int >("epoch"));
    item->setVersion(query.get< std::string >("version"));
    item->setRelease(query.get< std::string >("release"));
    item->setArch(query.get< std::string >("arch"));
    return trans_item;
}

std::vector< TransactionItemPtr >
RPMItem::getTransactionItems(libdnf::utils::SQLite3Ptr conn, int64_t transaction_id)
{
    std::vector< TransactionItemPtr > result;

    const char *sql =
        "SELECT "
        // trans_item
        "  ti.id, "
        "  ti.action, "
        "  ti.reason, "
        "  ti.state, "
        // repo
        "  r.repoid, "
        // rpm
        "  i.item_id, "
        "  i.name, "
        "  i.epoch, "
        "  i.version, "
        "  i.release, "
        "  i.arch "
        "FROM "
        "  trans_item ti, "
        "  repo r, "
        "  rpm i "
        "WHERE "
        "  ti.trans_id = ? "
        "  AND ti.repo_id = r.id "
        "  AND ti.item_id = i.item_id";
    libdnf::utils::SQLite3::Query query(*conn.get(), sql);
    query.bindv(transaction_id);

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        result.push_back(transactionItemFromQuery(conn, query, transaction_id));
    }
    return result;
}

std::string
RPMItem::getNEVRA() const
{
    // TODO: use string formatting
    if (epoch > 0) {
        return name + "-" + std::to_string(epoch) + ":" + version + "-" + release + "." + arch;
    }
    return name + "-" + version + "-" + release + "." + arch;
}

std::string
RPMItem::toStr() const
{
    return getNEVRA();
}

void
RPMItem::dbSelectOrInsert()
{
    const char *sql =
        "SELECT "
        "  item_id "
        "FROM "
        "  rpm "
        "WHERE "
        "  name = ? "
        "  AND epoch = ? "
        "  AND version = ? "
        "  AND release = ? "
        "  AND arch = ?";

    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);

    query.bindv(getName(), getEpoch(), getVersion(), getRelease(), getArch());
    libdnf::utils::SQLite3::Statement::StepResult result = query.step();

    if (result == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        setId(query.get< int >(0));
    } else {
        // insert and get the ID back
        dbInsert();
    }
}

TransactionItemPtr
RPMItem::getTransactionItem(libdnf::utils::SQLite3Ptr conn, const std::string &nevra)
{
    libdnf::rpm::Nevra nevraObject;
    if (!nevraObject.parse(nevra.c_str(), libdnf::rpm::Nevra::Form::NEVRA)) {
        return nullptr;
    }
    // TODO: hy_nevra_possibility should set epoch to 0 if epoch is not specified and libdnf::rpm::Nevra::Form::NEVRA
    // is used
    if (nevraObject.get_epoch().empty()) {
        nevraObject.set_epoch("0");
    }

    const char *sql = R"**(
        SELECT
            ti.trans_id,
            ti.id,
            ti.action,
            ti.reason,
            ti.state,
            r.repoid,
            i.item_id,
            i.name,
            i.epoch,
            i.version,
            i.release,
            i.arch
        FROM
            trans_item ti,
            repo r,
            rpm i
        WHERE
            ti.repo_id = r.id
            AND ti.item_id = i.item_id
            AND i.name = ?
            AND i.epoch = ?
            AND i.version = ?
            AND i.release = ?
            AND i.arch = ?
        ORDER BY
           ti.id DESC
        LIMIT 1
    )**";
    libdnf::utils::SQLite3::Query query(*conn, sql);
    query.bindv(nevraObject.get_name(),
                nevraObject.get_epoch(),
                nevraObject.get_version(),
                nevraObject.get_release(),
                nevraObject.get_arch());
    if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        return transactionItemFromQuery(conn, query, query.get< int64_t >("trans_id"));
    }
    return nullptr;
}

TransactionItemReason
RPMItem::resolveTransactionItemReason(libdnf::utils::SQLite3Ptr conn,
                                      const std::string &name,
                                      const std::string &arch,
                                      [[maybe_unused]] int64_t maxTransactionId)
{
    const char *sql = R"**(
        SELECT
            ti.action as action,
            ti.reason as reason
        FROM
            trans_item ti
        JOIN
            trans t ON ti.trans_id = t.id
        JOIN
            rpm i USING (item_id)
        WHERE
            t.state = 1
            /* see comment in TransactionItem.hpp - TransactionItemAction */
            AND ti.action not in (3, 5, 7, 10)
            AND i.name = ?
            AND i.arch = ?
        ORDER BY
            ti.trans_id DESC
        LIMIT 1
    )**";

    if (arch != "") {
        libdnf::utils::SQLite3::Query query(*conn, sql);
        query.bindv(name, arch);

        if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
            auto action = static_cast< TransactionItemAction >(query.get< int64_t >("action"));
            if (action == TransactionItemAction::REMOVE) {
                return TransactionItemReason::UNKNOWN;
            }
            auto reason = static_cast< TransactionItemReason >(query.get< int64_t >("reason"));
            return reason;
        }
    } else {
        const char *arch_sql = R"**(
            SELECT DISTINCT
                arch
            FROM
                rpm
            WHERE
                name = ?
        )**";

        libdnf::utils::SQLite3::Query arch_query(*conn, arch_sql);
        arch_query.bindv(name);

        TransactionItemReason result = TransactionItemReason::UNKNOWN;

        while (arch_query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
            auto rpm_arch = arch_query.get< std::string >("arch");

            libdnf::utils::SQLite3::Query query(*conn, sql);
            query.bindv(name, rpm_arch);
            while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
                auto action = static_cast< TransactionItemAction >(query.get< int64_t >("action"));
                if (action == TransactionItemAction::REMOVE) {
                    continue;
                }
                auto reason = static_cast< TransactionItemReason >(query.get< int64_t >("reason"));
                if (reason > result) {
                    result = reason;
                }
            }
        }
        return result;
    }
    return TransactionItemReason::UNKNOWN;
}

/**
 * Compare RPM packages
 * This method doesn't care about compare package names
 * \param other RPMItem to compare with
 * \return true if other package is newer (has higher version and/or epoch)
 */
bool
RPMItem::operator<(const RPMItem &other) const
{
    // compare epochs
    int32_t epochDif = other.getEpoch() - getEpoch();
    if (epochDif > 0) {
        return true;
    } else if (epoch < 0) {
        return false;
    }

    // compare versions
    std::stringstream versionThis(getVersion());
    std::stringstream versionOther(other.getVersion());

    std::string bufferThis;
    std::string bufferOther;
    while (std::getline(versionThis, bufferThis, '.') &&
           std::getline(versionOther, bufferOther, '.')) {
        int subVersionThis = std::stoi(bufferThis);
        int subVersionOther = std::stoi(bufferOther);
        if (subVersionThis == subVersionOther) {
            continue;
        }
        return subVersionOther > subVersionThis;
    }
    return false;
}

std::vector< int64_t >
RPMItem::searchTransactions(libdnf::utils::SQLite3Ptr conn, const std::vector< std::string > &patterns)
{
    std::vector< int64_t > result;

    const char *sql = R"**(
        SELECT DISTINCT
            t.id
        FROM
            trans t
        JOIN
            trans_item ti ON ti.trans_id = t.id
        JOIN
            rpm i USING (item_id)
        WHERE
            t.state = 1
            AND (
                i.name = ?
                OR i.epoch = ?
                OR i.version = ?
                OR i.release = ?
                OR i.arch = ?
            )
        ORDER BY
           trans_id DESC
    )**";
    libdnf::utils::SQLite3::Query query(*conn, sql);
    for (auto pattern : patterns) {
        query.bindv(pattern, pattern, pattern, pattern, pattern);
        while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
            result.push_back(query.get< int64_t >("id"));
        }
    }
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    return result;
}

}  // namespace libdnf::transaction
