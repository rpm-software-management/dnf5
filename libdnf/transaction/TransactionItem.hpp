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

#ifndef LIBDNF_TRANSACTION_TRANSACTIONITEM_HPP
#define LIBDNF_TRANSACTION_TRANSACTIONITEM_HPP

#include <memory>
#include <string>

#include "libdnf/utils/sqlite3/sqlite3.hpp"

namespace libdnf {
class TransactionItem;
typedef std::shared_ptr< TransactionItem > TransactionItemPtr;
}

#include "Item.hpp"
#include "CompsEnvironmentItem.hpp"
#include "CompsGroupItem.hpp"
#include "RPMItem.hpp"
#include "private/Repo.hpp"
#include "Transaction.hpp"
#include "Types.hpp"

namespace libdnf {

class TransactionItemBase {
public:
    ItemPtr getItem() const noexcept { return item; }
    void setItem(ItemPtr value) { item = value; }

    // typed items - workaround for lack of shared_ptr<> downcast support in SWIG
    CompsEnvironmentItemPtr getCompsEnvironmentItem() const noexcept
    {
        return std::dynamic_pointer_cast< CompsEnvironmentItem >(item);
    }
    CompsGroupItemPtr getCompsGroupItem() const noexcept
    {
        return std::dynamic_pointer_cast< CompsGroupItem >(item);
    }
    RPMItemPtr getRPMItem() const noexcept { return std::dynamic_pointer_cast< RPMItem >(item); }

    const std::string &getRepoid() const noexcept { return repoid; }
    void setRepoid(const std::string &value) { repoid = value; }

    TransactionItemAction getAction() const noexcept { return action; }
    void setAction(TransactionItemAction value) { action = value; }

    TransactionItemReason getReason() const noexcept { return reason; }
    void setReason(TransactionItemReason value) { reason = value; }

    const std::string &getActionName();
    const std::string &getActionShort();

    TransactionItemState getState() const noexcept { return state; }
    void setState(TransactionItemState value) { state = value; }

    /**
     * @brief Has the item appeared on the system during the transaction?
     *
     * @return bool
     */
    bool isForwardAction() const;

    /**
     * @brief Has the item got removed from the system during the transaction?
     *
     * @return bool
     */
    bool isBackwardAction() const;

protected:
    ItemPtr item;
    std::string repoid;
    TransactionItemAction action = TransactionItemAction::INSTALL;
    TransactionItemReason reason = TransactionItemReason::UNKNOWN;
    TransactionItemState state = TransactionItemState::UNKNOWN;
};

typedef std::shared_ptr< TransactionItemBase > TransactionItemBasePtr;

class TransactionItem : public TransactionItemBase {
public:
    explicit TransactionItem(Transaction *trans);

    TransactionItem(libdnf::utils::SQLite3Ptr conn, int64_t transID);

    int64_t getId() const noexcept { return id; }
    void setId(int64_t value) { id = value; }

    uint32_t getInstalledBy() const;

    // int64_t getTransactionId() const noexcept { return trans.getId(); }

    const std::vector< TransactionItemPtr > &getReplacedBy() const noexcept { return replacedBy; }
    void addReplacedBy(TransactionItemPtr value) { if (value) replacedBy.push_back(value); }

    void save();
    void saveReplacedBy();
    void saveState();

    std::size_t getHash() { return reinterpret_cast< std::size_t >(this); }
    bool operator==(TransactionItem & other) { return (other.getHash() == getHash()); }
    bool operator==(TransactionItemPtr other) { return (other->getHash() == getHash()); }

    // needed for sorting a list of transaction items in Python
    // TODO: consider replacing trivial string comparison with something better
    bool operator<(TransactionItem & other) { return (getItem()->toStr() < other.getItem()->toStr()); }
    bool operator<(TransactionItemPtr other) { return (getItem()->toStr() < other->getItem()->toStr()); }

protected:
    int64_t id = 0;
    Transaction *trans;

    const int64_t transID;
    libdnf::utils::SQLite3Ptr conn;

    // TODO: replace with objects? it's just repoid, probably not necessary
    std::vector< TransactionItemPtr > replacedBy;

    void dbInsert();
    void dbUpdate();
};

} // namespace libdnf

#endif // LIBDNF_TRANSACTION_TRANSACTIONITEM_HPP
