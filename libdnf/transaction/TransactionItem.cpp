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

#include "TransactionItem.hpp"

namespace libdnf {

// TODO: translations
static const std::map< TransactionItemAction, std::string > transactionItemActionName = {
    {TransactionItemAction::INSTALL, "Install"},
    {TransactionItemAction::DOWNGRADE, "Downgrade"},
    {TransactionItemAction::DOWNGRADED, "Downgraded"},
    {TransactionItemAction::OBSOLETE, "Obsolete"},
    {TransactionItemAction::OBSOLETED, "Obsoleted"},
    {TransactionItemAction::UPGRADE, "Upgrade"},
    {TransactionItemAction::UPGRADED, "Upgraded"},
    {TransactionItemAction::REMOVE, "Removed"},
    {TransactionItemAction::REINSTALL, "Reinstall"},
    {TransactionItemAction::REINSTALLED, "Reinstalled"},
    {TransactionItemAction::REASON_CHANGE, "Reason Change"},
};

static const std::map< TransactionItemAction, std::string > transactionItemActionShort = {
    {TransactionItemAction::INSTALL, "I"},
    {TransactionItemAction::DOWNGRADE, "D"},
    {TransactionItemAction::DOWNGRADED, "D"},
    {TransactionItemAction::OBSOLETE, "O"},
    {TransactionItemAction::OBSOLETED, "O"},
    {TransactionItemAction::UPGRADE, "U"},
    {TransactionItemAction::UPGRADED, "U"},
    // "R" is for Reinstall, therefore use "E" for rEmove (or Erase)
    {TransactionItemAction::REMOVE, "E"},
    {TransactionItemAction::REINSTALL, "R"},
    {TransactionItemAction::REINSTALLED, "R"},
    // TODO: replace "?" with something better
    {TransactionItemAction::REASON_CHANGE, "?"},
};

/*
static const std::map<std::string, TransactionItemReason> nameTransactionItemReason = {
    {, "I"},
    {TransactionItemAction::DOWNGRADE, "D"},
    {TransactionItemAction::DOWNGRADED, "D"},
    {TransactionItemAction::OBSOLETE, "O"},
    {TransactionItemAction::OBSOLETED, "O"},
    {TransactionItemAction::UPGRADE, "U"},
    {TransactionItemAction::UPGRADED, "U"},
    // "R" is for Reinstall, therefore use "E" for rEmove (or Erase)
    {TransactionItemAction::REMOVE, "E"},
    {TransactionItemAction::REINSTALL, "R"},
};
TransactionItemReason to_TransactionItemReason(const std::string & s) {
    return
*/

const std::string &
TransactionItemBase::getActionName()
{
    return transactionItemActionName.at(getAction());
}

const std::string &
TransactionItemBase::getActionShort()
{
    return transactionItemActionShort.at(getAction());
}

TransactionItem::TransactionItem(Transaction *trans)
  : trans(trans)
  , transID(0)
  , conn(trans->conn)
{
}

bool
TransactionItemBase::isForwardAction() const
{
    switch (action) {
        case TransactionItemAction::INSTALL:
        case TransactionItemAction::DOWNGRADE:
        case TransactionItemAction::OBSOLETE:
        case TransactionItemAction::UPGRADE:
        case TransactionItemAction::REINSTALL:
            return true;
        default:
            return false;
    }
}

bool
TransactionItemBase::isBackwardAction() const
{
    switch (action) {
        case TransactionItemAction::REMOVE:
        case TransactionItemAction::DOWNGRADED:
        case TransactionItemAction::OBSOLETED:
        case TransactionItemAction::UPGRADED:
        case TransactionItemAction::REINSTALLED:
            return true;
        default:
            return false;
    }
}

TransactionItem::TransactionItem(libdnf::utils::SQLite3Ptr conn, int64_t transID)
  : trans(nullptr)
  , transID(transID)
  , conn(conn)
{
}

void
TransactionItem::save()
{
    getItem()->save();
    if (getId() == 0) {
        dbInsert();
    } else {
        dbUpdate();
    }
}

void
TransactionItem::dbInsert()
{
    if (trans == nullptr) {
        throw std::runtime_error(
            _("Attempt to insert transaction item into completed transaction"));
    }

    const char *sql = R"**(
        INSERT INTO
          trans_item (
            id,
            trans_id,
            item_id,
            repo_id,
            action,
            reason,
            state
          )
        VALUES
          (null, ?, ?, ?, ?, ?, ?)
    )**";

    // save the transaction item
    libdnf::utils::SQLite3::Statement query(*(conn.get()), sql);
    query.bindv(trans->getId(),
                getItem()->getId(),
                swdb_private::Repo::getCached(conn, getRepoid())->getId(),
                static_cast< int >(getAction()),
                static_cast< int >(getReason()),
                static_cast< int >(getState()));
    query.step();
    setId(conn->last_insert_rowid());
}

void
TransactionItem::saveReplacedBy()
{
    if (replacedBy.empty()) {
        return;
    }
    const char *sql = "INSERT OR REPLACE INTO item_replaced_by VALUES (?, ?)";
    libdnf::utils::SQLite3::Statement replacedByQuery(*(conn.get()), sql);
    bool first = true;
    for (const auto &newItem : replacedBy) {
        if (!first) {
            // reset the prepared statement, so it can be executed again
            replacedByQuery.reset();
        }
        replacedByQuery.bindv(getId(), newItem->getId());
        replacedByQuery.step();
        first = false;
    }
}

void
TransactionItem::saveState()
{
    const char *sql = R"**(
        UPDATE
          trans_item
        SET
          state = ?
        WHERE
          id = ?
    )**";

    libdnf::utils::SQLite3::Statement query(*conn, sql);
    query.bindv(static_cast< int >(getState()), getId());
    query.step();
}

void
TransactionItem::dbUpdate()
{
    if (trans == nullptr) {
        throw std::runtime_error(_("Attempt to update transaction item in completed transaction"));
    }

    const char *sql = R"**(
        UPDATE
          trans_item
        SET
          trans_id=?,
          item_id=?,
          repo_id=?,
          action=?,
          reason=?,
          state=?
        WHERE
          id = ?
    )**";

    libdnf::utils::SQLite3::Statement query(*(conn.get()), sql);
    query.bindv(trans->getId(),
                getItem()->getId(),
                swdb_private::Repo::getCached(trans->conn, getRepoid())->getId(),
                static_cast< int >(getAction()),
                static_cast< int >(getReason()),
                static_cast< int >(getState()),
                getId());
    query.step();
}

uint32_t
TransactionItem::getInstalledBy() const {
    if (!trans) {
        // null pointer -> create a local instance to return the user id
        Transaction t(conn, transID);
        return t.getUserId();
    }
    return trans->getUserId();
}

} // namespace libdnf
