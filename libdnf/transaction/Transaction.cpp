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

#include "Transaction.hpp"
#include "CompsEnvironmentItem.hpp"
#include "CompsGroupItem.hpp"
#include "RPMItem.hpp"
#include "TransactionItem.hpp"

#include "libdnf/utils/bgettext/bgettext-lib.h"

#include <fmt/format.h>


namespace libdnf {

Transaction::Transaction(libdnf::utils::SQLite3Ptr conn, int64_t pk)
  : conn{conn}
{
    dbSelect(pk);
}

Transaction::Transaction(libdnf::utils::SQLite3Ptr conn)
  : conn{conn}
{
}

bool
Transaction::operator==(const Transaction &other) const
{
    return getId() == other.getId() && getDtBegin() == other.getDtBegin() &&
           getRpmdbVersionBegin() == other.getRpmdbVersionBegin();
}

/**
 * Compare two transactions on:
 *  transaction ID
 *  begin timestamp
 *  packages in the system at the time of transaction
 * \param other transaction to compare with
 * \return true if other transaction is older
 */
bool
Transaction::operator<(const Transaction &other) const
{
    return getId() > other.getId() || getDtBegin() > other.getDtBegin() ||
           getRpmdbVersionBegin() > other.getRpmdbVersionBegin();
}

/**
 * \param other transaction to compare with
 * \return true if other transaction is newer
 */
bool
Transaction::operator>(const Transaction &other) const
{
    return getId() < other.getId() || getDtBegin() < other.getDtBegin() ||
           getRpmdbVersionBegin() < other.getRpmdbVersionBegin();
}

void
Transaction::dbSelect(int64_t pk)
{
    const char *sql =
        "SELECT "
        "  dt_begin, "
        "  dt_end, "
        "  rpmdb_version_begin, "
        "  rpmdb_version_end, "
        "  releasever, "
        "  user_id, "
        "  cmdline, "
        "  state "
        "FROM "
        "  trans "
        "WHERE "
        "  id = ?";
    libdnf::utils::SQLite3::Query query(*conn.get(), sql);
    query.bindv(pk);
    query.step();

    id = pk;
    dtBegin = query.get< int >("dt_begin");
    dtEnd = query.get< int >("dt_end");
    rpmdbVersionBegin = query.get< std::string >("rpmdb_version_begin");
    rpmdbVersionEnd = query.get< std::string >("rpmdb_version_end");
    releasever = query.get< std::string >("releasever");
    userId = query.get< uint32_t >("user_id");
    cmdline = query.get< std::string >("cmdline");
    state = static_cast< TransactionState >(query.get< int >("state"));
}

/**
 * Loader for the transaction items.
 * \return list of transaction items associated with the transaction
 */
std::vector< TransactionItemPtr >
Transaction::getItems()
{
    if (!items.empty()) {
        return items;
    }
    std::vector< TransactionItemPtr > result;
    auto rpms = RPMItem::getTransactionItems(conn, getId());
    result.insert(result.end(), rpms.begin(), rpms.end());

    auto comps_groups = CompsGroupItem::getTransactionItems(conn, getId());
    result.insert(result.end(), comps_groups.begin(), comps_groups.end());

    auto comps_environments = CompsEnvironmentItem::getTransactionItems(conn, getId());
    result.insert(result.end(), comps_environments.begin(), comps_environments.end());

    return result;
}

/**
 * Load list of software performed with for current transaction from the database.
 * Transaction has to be saved in advance, otherwise empty list will be returned.
 * \return list of RPMItem objects that performed the transaction
 */
const std::set< std::shared_ptr< RPMItem > >
Transaction::getSoftwarePerformedWith() const
{
    const char *sql = R"**(
        SELECT
            item_id
        FROM
            trans_with
        WHERE
            trans_id = ?
    )**";

    std::set< std::shared_ptr< RPMItem > > software;

    libdnf::utils::SQLite3::Query query(*conn.get(), sql);
    query.bindv(getId());

    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        software.insert(std::make_shared< RPMItem >(conn, query.get< int64_t >("item_id")));
    }

    return software;
}

std::vector< std::pair< int, std::string > >
Transaction::getConsoleOutput() const
{
    const char *sql = R"**(
        SELECT
            file_descriptor,
            line
        FROM
            console_output
        WHERE
            trans_id = ?
        ORDER BY
            id
    )**";
    libdnf::utils::SQLite3::Query query(*conn, sql);
    query.bindv(getId());
    std::vector< std::pair< int, std::string > > result;
    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto fileDescriptor = query.get< int >("file_descriptor");
        auto line = query.get< std::string >("line");
        result.push_back(std::make_pair(fileDescriptor, line));
    }
    return result;
}

void
Transaction::begin()
{
    if (id != 0) {
        throw std::runtime_error(_("Transaction has already began!"));
    }
    dbInsert();
    saveItems();
}

void
Transaction::finish(TransactionState state)
{
    // save states to the database before checking for UNKNOWN state
    for (auto i : getItems()) {
        i->saveState();
    }

    for (auto i : getItems()) {
        if (i->getState() == TransactionItemState::UNKNOWN) {
            throw std::runtime_error(
                fmt::format(_("TransactionItem state is not set: {}"), i->getItem()->toStr()));
        }
    }

    setState(state);
    dbUpdate();
}

void
Transaction::dbInsert()
{
    const char *sql =
        "INSERT INTO "
        "  trans ("
        "    dt_begin, "
        "    dt_end, "
        "    rpmdb_version_begin, "
        "    rpmdb_version_end, "
        "    releasever, "
        "    user_id, "
        "    cmdline, "
        "    state, "
        "    id "
        "  ) "
        "VALUES "
        "  (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(getDtBegin(),
                getDtEnd(),
                getRpmdbVersionBegin(),
                getRpmdbVersionEnd(),
                getReleasever(),
                getUserId(),
                getCmdline(),
                static_cast< int >(getState()));
    if (getId() > 0) {
        query.bind(9, getId());
    }
    query.step();
    setId(conn->last_insert_rowid());

    // add used software - has to be added at initialization state
    if (!softwarePerformedWith.empty()) {
        sql = R"**(
            INSERT OR REPLACE INTO
                trans_with (
                    trans_id,
                    item_id
                )
            VALUES
                (?, ?)
        )**";
        libdnf::utils::SQLite3::Statement swQuery(*conn.get(), sql);
        bool first = true;
        for (auto software : softwarePerformedWith) {
            if (!first) {
                swQuery.reset();
            }
            first = false;
            // save the item to create a database id
            software->save();
            swQuery.bindv(getId(), software->getId());
            swQuery.step();
        }
    }
}

void
Transaction::dbUpdate()
{
    const char *sql =
        "UPDATE "
        "  trans "
        "SET "
        "  dt_begin=?, "
        "  dt_end=?, "
        "  rpmdb_version_begin=?, "
        "  rpmdb_version_end=?, "
        "  releasever=?, "
        "  user_id=?, "
        "  cmdline=?, "
        "  state=? "
        "WHERE "
        "  id = ?";
    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(getDtBegin(),
                getDtEnd(),
                getRpmdbVersionBegin(),
                getRpmdbVersionEnd(),
                getReleasever(),
                getUserId(),
                getCmdline(),
                static_cast< int >(getState()),
                getId());
    query.step();
}

TransactionItemPtr
Transaction::addItem(std::shared_ptr< Item > item,
                                           const std::string &repoid,
                                           TransactionItemAction action,
                                           TransactionItemReason reason)
{
    for (auto & i : items) {
        if (i->getItem()->toStr() != item->toStr()) {
            continue;
        }
        if (i->getRepoid() != repoid) {
            continue;
        }
        if (i->getAction() != action) {
            continue;
        }
        if (reason > i->getReason()) {
            // use the more significant reason
            i->setReason(reason);
        }
        // don't add duplicates to the list
        // return an existing transaction item if exists
        return i;
    }
    auto trans_item = std::make_shared< TransactionItem >(this);
    trans_item->setItem(item);
    trans_item->setRepoid(repoid);
    trans_item->setAction(action);
    trans_item->setReason(reason);
    items.push_back(trans_item);
    return trans_item;
}

void
Transaction::saveItems()
{
    // TODO: remove all existing items from the database first?
    for (auto i : items) {
        i->save();
    }

    /* this has to be done in a separate loop to make sure
     * that all the items already have ID assigned
     */
    for (auto i : items) {
        i->saveReplacedBy();
    }
}


/**
 * Append software to softwarePerformedWith list.
 * Software is saved to the database using save method and therefore
 * all the software has to be added before transaction is saved.
 * \param software RPMItem used to perform the transaction
 */
void
Transaction::addSoftwarePerformedWith(std::shared_ptr< RPMItem > software)
{
    softwarePerformedWith.insert(software);
}

/**
 * Save console output line for current transaction to the database. Transaction has
 *  to be saved in advance, otherwise an exception will be thrown.
 * \param fileDescriptor UNIX file descriptor index (1 = stdout, 2 = stderr).
 * \param line console output content
 */
void
Transaction::addConsoleOutputLine(int fileDescriptor, const std::string &line)
{
    if (!getId()) {
        throw std::runtime_error(_("Can't add console output to unsaved transaction"));
    }

    const char *sql = R"**(
        INSERT INTO
            console_output (
                trans_id,
                file_descriptor,
                line
            )
        VALUES
            (?, ?, ?);
    )**";
    libdnf::utils::SQLite3::Statement query(*conn, sql);
    query.bindv(getId(), fileDescriptor, line);
    query.step();
}


} // namespace libdnf
