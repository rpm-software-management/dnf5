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


#include "libdnf/utils/bgettext/bgettext-lib.h"
#include <fmt/format.h>
//#include "../../utils/tinyformat/tinyformat.hpp"

#include "../CompsEnvironmentItem.hpp"
#include "../CompsGroupItem.hpp"
#include "../RPMItem.hpp"
#include "../TransactionItem.hpp"
#include "Transaction.hpp"


namespace libdnf {

swdb_private::Transaction::Transaction(libdnf::utils::SQLite3Ptr conn)
  : libdnf::Transaction(conn)
{
}

void
swdb_private::Transaction::begin()
{
    if (id != 0) {
        throw std::runtime_error(_("Transaction has already began!"));
    }
    dbInsert();
    saveItems();
}

void
swdb_private::Transaction::finish(TransactionState state)
{
    // save states to the database before checking for UNKNOWN state
    for (auto i : getItems()) {
        i->saveState();
    }

    for (auto i : getItems()) {
        if (i->getState() == TransactionItemState::UNKNOWN) {
            throw std::runtime_error(
                fmt::format(_("TransactionItem state is not set: %s"), i->getItem()->toStr()));
        }
    }

    setState(state);
    dbUpdate();
}

void
swdb_private::Transaction::dbInsert()
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
swdb_private::Transaction::dbUpdate()
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
swdb_private::Transaction::addItem(std::shared_ptr< Item > item,
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
swdb_private::Transaction::saveItems()
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
 * Loader for the transaction items.
 * \return list of transaction items associated with the transaction
 */
std::vector< TransactionItemPtr >
swdb_private::Transaction::getItems()
{
    if (items.empty()) {
        items = libdnf::Transaction::getItems();
    }
    return items;
}

/**
 * Append software to softwarePerformedWith list.
 * Software is saved to the database using save method and therefore
 * all the software has to be added before transaction is saved.
 * \param software RPMItem used to perform the transaction
 */
void
swdb_private::Transaction::addSoftwarePerformedWith(std::shared_ptr< RPMItem > software)
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
swdb_private::Transaction::addConsoleOutputLine(int fileDescriptor, const std::string &line)
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
