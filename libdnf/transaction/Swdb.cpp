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

#include <cstdio>
#include <solv/bitmap.h>
#include <solv/solvable.h>

#include "../hy-query-private.hpp"
#include "../hy-subject.h"
#include "../nevra.hpp"

#include "../sack/packageset.hpp"

#include "../utils/bgettext/bgettext-lib.h"
#include "../utils/filesystem.hpp"
#include "../utils/sqlite3/Sqlite3.hpp"

#include "RPMItem.hpp"
#include "Swdb.hpp"
#include "Transformer.hpp"

namespace libdnf {

Swdb::Swdb(SQLite3Ptr conn)
  : conn{conn}
  , autoClose(true)
{
}

Swdb::Swdb(SQLite3Ptr conn, bool autoClose)
  : conn{conn}
  , autoClose(autoClose)
{
}

Swdb::Swdb(const std::string &path)
  : conn(nullptr)
  , autoClose(true)
{
    if (path == ":memory:") {
        // writing to an in-memory database
        conn = std::make_shared< SQLite3 >(path);
        Transformer::createDatabase(conn);
    } else if (!pathExists(path.c_str())) {
        // writing to a file that doesn't exist and must be created

        // extract persistdir from path - "/var/lib/dnf/"
        auto found = path.find_last_of("/");

        Transformer transformer(path.substr(0, found), path);
        transformer.transform();

        conn = std::make_shared< SQLite3 >(path);
    } else {
        // writing to an existing file
        conn = std::make_shared< SQLite3 >(path);
    }
}

void
Swdb::resetDatabase()
{
    conn->close();
    if (pathExists(getPath().c_str())) {
        remove(getPath().c_str());
    }
    conn->open();
    Transformer::createDatabase(conn);
}

void
Swdb::closeDatabase()
{
    conn->close();
}

Swdb::~Swdb()
{
    if (autoClose) {
        try {
            closeDatabase();
        } catch(const std::exception &){}
    }
}

void
Swdb::initTransaction()
{
    if (transactionInProgress) {
        throw std::logic_error(_("In progress"));
    }
    transactionInProgress = std::unique_ptr< swdb_private::Transaction >(
        new swdb_private::Transaction(conn));
    itemsInProgress.clear();
}

int64_t
Swdb::beginTransaction(int64_t dtBegin,
                       std::string rpmdbVersionBegin,
                       std::string cmdline,
                       uint32_t userId)
{
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }

    // begin transaction
    transactionInProgress->setDtBegin(dtBegin);
    transactionInProgress->setRpmdbVersionBegin(rpmdbVersionBegin);
    transactionInProgress->setCmdline(cmdline);
    transactionInProgress->setUserId(userId);
    transactionInProgress->begin();

    // save rpm items to map to resolve RPM callbacks
    for (auto item : transactionInProgress->getItems()) {
        auto transItem = item->getItem();
        if (transItem->getItemType() != ItemType::RPM) {
            continue;
        }
        auto rpmItem = std::dynamic_pointer_cast< RPMItem >(transItem);
        itemsInProgress[rpmItem->getNEVRA()] = item;
    }

    return transactionInProgress->getId();
}

int64_t
Swdb::endTransaction(int64_t dtEnd, std::string rpmdbVersionEnd, TransactionState state)
{
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    transactionInProgress->setDtEnd(dtEnd);
    transactionInProgress->setRpmdbVersionEnd(rpmdbVersionEnd);
    transactionInProgress->finish(state);
    return transactionInProgress->getId();
}

int64_t
Swdb::closeTransaction()
{
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    int64_t result = transactionInProgress->getId();
    transactionInProgress = std::unique_ptr< swdb_private::Transaction >(nullptr);
    itemsInProgress.clear();
    return result;
}


TransactionItemPtr
Swdb::addItem(std::shared_ptr< Item > item,
              const std::string &repoid,
              TransactionItemAction action,
              TransactionItemReason reason)
//            std::shared_ptr<TransactionItem> replacedBy)
{
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    // auto replacedBy = std::make_shared<TransactionItem>(nullptr);
    return transactionInProgress->addItem(item, repoid, action, reason);
}

void
Swdb::setItemDone(const std::string &nevra)
{
    if (!transactionInProgress) {
        throw std::logic_error(_("No transaction in progress"));
    }
    auto item = itemsInProgress[nevra];
    item->setState(TransactionItemState::DONE);
    item->saveState();
}

TransactionItemReason
Swdb::resolveRPMTransactionItemReason(const std::string &name,
                                      const std::string &arch,
                                      int64_t maxTransactionId)
{
    // TODO:
    // -1: latest
    // -2: latest and lastTransaction data in memory
    if (maxTransactionId == -2 && transactionInProgress != nullptr) {
        for (auto i : transactionInProgress->getItems()) {
            auto rpm = std::dynamic_pointer_cast< RPMItem >(i->getItem());
            if (!rpm) {
                continue;
            }
            if (rpm->getName() == name && rpm->getArch() == arch) {
                return i->getReason();
            }
        }
    }

    return RPMItem::resolveTransactionItemReason(conn, name, arch, maxTransactionId);
}

const std::string
Swdb::getRPMRepo(const std::string &nevra)
{
    Nevra nevraObject;
    if (!nevraObject.parse(nevra.c_str(), HY_FORM_NEVRA)) {
        return "";
    }
    // TODO: hy_nevra_possibility should set epoch to 0 if epoch is not specified
    // and HY_FORM_NEVRA is used
    if (nevraObject.getEpoch() < 0) {
        nevraObject.setEpoch(0);
    }

    const char *sql = R"**(
        SELECT
            repo.repoid as repoid
        FROM
            trans_item ti
        JOIN
            rpm USING (item_id)
        JOIN
            repo ON ti.repo_id == repo.id
        WHERE
            ti.action not in (3, 5, 7, 10)
            AND rpm.name = ?
            AND rpm.epoch = ?
            AND rpm.version = ?
            AND rpm.release = ?
            AND rpm.arch = ?
        ORDER BY
            ti.id DESC
        LIMIT 1;
    )**";
    // TODO: where trans.done != 0
    SQLite3::Query query(*conn, sql);
    query.bindv(nevraObject.getName(),
                nevraObject.getEpoch(),
                nevraObject.getVersion(),
                nevraObject.getRelease(),
                nevraObject.getArch());
    if (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto repoid = query.get< std::string >("repoid");
        return repoid;
    }
    return "";
}

TransactionItemPtr
Swdb::getRPMTransactionItem(const std::string &nevra)
{
    return RPMItem::getTransactionItem(conn, nevra);
}

TransactionPtr
Swdb::getLastTransaction()
{
    const char *sql = R"**(
        SELECT
            id
        FROM
            trans
        ORDER BY
            id DESC
        LIMIT 1
    )**";
    SQLite3::Statement query(*conn, sql);
    if (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto transId = query.get< int64_t >(0);
        auto transaction = std::make_shared< Transaction >(conn, transId);
        return transaction;
    }
    return nullptr;
}

std::vector< TransactionPtr >
Swdb::listTransactions()
{
    const char *sql = R"**(
        SELECT
            id
        FROM
            trans
        ORDER BY
            id
    )**";
    SQLite3::Statement query(*conn, sql);
    std::vector< TransactionPtr > result;
    while (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto transId = query.get< int64_t >(0);
        auto transaction = std::make_shared< Transaction >(conn, transId);
        result.push_back(transaction);
    }
    return result;
}

void
Swdb::setReleasever(std::string value)
{
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    transactionInProgress->setReleasever(value);
}


void
Swdb::addConsoleOutputLine(int fileDescriptor, std::string line)
{
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    transactionInProgress->addConsoleOutputLine(fileDescriptor, line);
}

TransactionItemPtr
Swdb::getCompsGroupItem(const std::string &groupid)
{
    return CompsGroupItem::getTransactionItem(conn, groupid);
}

std::vector< TransactionItemPtr >
Swdb::getCompsGroupItemsByPattern(const std::string &pattern)
{
    return CompsGroupItem::getTransactionItemsByPattern(conn, pattern);
}

std::vector< std::string >
Swdb::getPackageCompsGroups(const std::string &packageName)
{
    const char *sql_all_groups = R"**(
        SELECT DISTINCT
            g.groupid
        FROM
            comps_group g
        JOIN
            comps_group_package p ON p.group_id = g.item_id
        WHERE
            p.name = ?
            AND p.installed = 1
        ORDER BY
            g.groupid
    )**";

    const char *sql_trans_items = R"**(
        SELECT
            ti.action as action,
            ti.reason as reason,
            i.item_id as group_id
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
        LIMIT 1
    )**";

    const char *sql_group_package = R"**(
        SELECT
            p.name
        FROM
            comps_group_package p
        WHERE
            p.group_id = ?
            AND p.installed = 1
    )**";

    std::vector< std::string > result;

    // list all relevant groups
    SQLite3::Query query_all_groups(*conn, sql_all_groups);
    query_all_groups.bindv(packageName);

    while (query_all_groups.step() == SQLite3::Statement::StepResult::ROW) {
        auto groupid = query_all_groups.get< std::string >("groupid");
        SQLite3::Query query_trans_items(*conn, sql_trans_items);
        query_trans_items.bindv(groupid);
        if (query_trans_items.step() == SQLite3::Statement::StepResult::ROW) {
            auto action =
                static_cast< TransactionItemAction >(query_trans_items.get< int64_t >("action"));
            // if the last record is group removal, skip
            if (action == TransactionItemAction::REMOVE) {
                continue;
            }
            auto groupId = query_trans_items.get< int64_t >("group_id");
            SQLite3::Query query_group_package(*conn, sql_group_package);
            query_group_package.bindv(groupId);
            if (query_group_package.step() == SQLite3::Statement::StepResult::ROW) {
                result.push_back(groupid);
            }
        }
    }
    return result;
}

std::vector< std::string >
Swdb::getCompsGroupEnvironments(const std::string &groupId)
{
    const char *sql_all_environments = R"**(
        SELECT DISTINCT
            e.environmentid
        FROM
            comps_environment e
        JOIN
            comps_environment_group g ON g.environment_id = e.item_id
        WHERE
            g.groupid = ?
            AND g.installed = 1
        ORDER BY
            e.environmentid
    )**";

    const char *sql_trans_items = R"**(
        SELECT
            ti.action as action,
            ti.reason as reason,
            i.item_id as environment_id
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

    const char *sql_environment_group = R"**(
        SELECT
            g.groupid
        FROM
            comps_environment_group g
        WHERE
            g.environment_id = ?
            AND g.installed = 1
    )**";

    std::vector< std::string > result;

    // list all relevant groups
    SQLite3::Query query_all_environments(*conn, sql_all_environments);
    query_all_environments.bindv(groupId);

    while (query_all_environments.step() == SQLite3::Statement::StepResult::ROW) {
        auto envid = query_all_environments.get< std::string >("environmentid");
        SQLite3::Query query_trans_items(*conn, sql_trans_items);
        query_trans_items.bindv(envid);
        if (query_trans_items.step() == SQLite3::Statement::StepResult::ROW) {
            auto action =
                static_cast< TransactionItemAction >(query_trans_items.get< int64_t >("action"));
            // if the last record is group removal, skip
            if (action == TransactionItemAction::REMOVE) {
                continue;
            }
            auto envId = query_trans_items.get< int64_t >("environment_id");
            SQLite3::Query query_environment_group(*conn, sql_environment_group);
            query_environment_group.bindv(envId);
            if (query_environment_group.step() == SQLite3::Statement::StepResult::ROW) {
                result.push_back(envid);
            }
        }
    }
    return result;
}

TransactionItemPtr
Swdb::getCompsEnvironmentItem(const std::string &envid)
{
    return CompsEnvironmentItem::getTransactionItem(conn, envid);
}

std::vector< TransactionItemPtr >
Swdb::getCompsEnvironmentItemsByPattern(const std::string &pattern)
{
    return CompsEnvironmentItem::getTransactionItemsByPattern(conn, pattern);
}

RPMItemPtr
Swdb::createRPMItem()
{
    return std::make_shared< RPMItem >(conn);
}

CompsEnvironmentItemPtr
Swdb::createCompsEnvironmentItem()
{
    return std::make_shared< CompsEnvironmentItem >(conn);
}

CompsGroupItemPtr
Swdb::createCompsGroupItem()
{
    return std::make_shared< CompsGroupItem >(conn);
}

/**
 * Filter unneeded packages from pool
 *
 * \return list of user installed package IDs
 */
void
Swdb::filterUserinstalled(PackageSet & installed) const
{
    Pool * pool = dnf_sack_get_pool(installed.getSack());

    // iterate over solvables
    Id id = -1;
    while ((id = installed.next(id)) != -1) {

        Solvable *s = pool_id2solvable(pool, id);
        const char *name = pool_id2str(pool, s->name);
        const char *arch = pool_id2str(pool, s->arch);

        auto reason = RPMItem::resolveTransactionItemReason(conn, name, arch, -1);
        // if not dep or weak, than consider it user installed
        if (reason == TransactionItemReason::DEPENDENCY ||
            reason == TransactionItemReason::WEAK_DEPENDENCY) {
            installed.remove(id);
        }
    }
}

std::vector< int64_t >
Swdb::searchTransactionsByRPM(const std::vector< std::string > &patterns)
{
    return RPMItem::searchTransactions(conn, patterns);
}

} // namespace libdnf
