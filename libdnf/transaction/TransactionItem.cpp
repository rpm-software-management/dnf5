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
#include "Types.hpp"
#include "transaction_item_action.hpp"


namespace libdnf::transaction {


std::string TransactionItemBase::getActionName() {
    return TransactionItemAction_get_name(action);
}


std::string TransactionItemBase::getActionShort() {
    return TransactionItemAction_get_short(action);
}


TransactionItem::TransactionItem(Transaction *trans)
  : trans(trans)
  , transID(0)
  , conn(trans->conn)
{
}


bool TransactionItemBase::isForwardAction() const {
    return TransactionItemAction_is_forward_action(action);
}


bool TransactionItemBase::isBackwardAction() const {
    return TransactionItemAction_is_backward_action(action);
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
    query.bindv(trans->get_id(),
                getItem()->getId(),
                Repo::getCached(conn, getRepoid())->getId(),
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
    query.bindv(trans->get_id(),
                getItem()->getId(),
                Repo::getCached(trans->conn, getRepoid())->getId(),
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
        return t.get_user_id();
    }
    return trans->get_user_id();
}

}  // namespace libdnf::transaction
