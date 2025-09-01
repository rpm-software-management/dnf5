// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_ITEM_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_ITEM_HPP

#include "transaction_item_action.hpp"
#include "transaction_item_reason.hpp"
#include "transaction_item_state.hpp"

#include "libdnf5/defs.h"

#include <string>


namespace libdnf5::transaction {

class Transaction;
class CompsEnvironment;
class CompsGroup;
class RpmDbUtils;
class Package;
class TransItemDbUtils;
class CompsGroupDbUtils;
class CompsEnvironmentDbUtils;
class CompsGroupPackageDbUtils;
class CompsEnvironmentGroupDbUtils;


class LIBDNF_API TransactionItem {
public:
    using Action = TransactionItemAction;
    using Reason = TransactionItemReason;
    using State = TransactionItemState;

    /// Get action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept;

    /// Get reason of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept;

    /// Get transaction item repoid (text identifier of a repository)
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getRepoid()
    const std::string & get_repoid() const noexcept;

    /// Get transaction item state
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept;

    ~TransactionItem();
    TransactionItem(const TransactionItem & src);
    TransactionItem & operator=(const TransactionItem & src);
    TransactionItem(TransactionItem && src) noexcept;
    TransactionItem & operator=(TransactionItem && src) noexcept;

private:
    friend RpmDbUtils;
    friend Transaction;
    friend CompsEnvironment;
    friend CompsGroup;
    friend Package;
    friend TransItemDbUtils;
    friend CompsGroupDbUtils;
    friend CompsEnvironmentDbUtils;
    friend CompsGroupPackageDbUtils;
    friend CompsEnvironmentGroupDbUtils;

    LIBDNF_LOCAL explicit TransactionItem(const Transaction & trans);

    /// Get database id (primary key) of the transaction item (table 'trans_item')
    LIBDNF_LOCAL int64_t get_id() const noexcept;

    /// Set database id (primary key) of the transaction item (table 'trans_item')
    LIBDNF_LOCAL void set_id(int64_t value);

    /// Set action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setAction(libdnf::TransactionItemAction value)
    LIBDNF_LOCAL void set_action(Action value);

    /// Get name of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getActionName()
    LIBDNF_LOCAL std::string get_action_name();

    /// Get abbreviated name of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getActionShort()
    LIBDNF_LOCAL std::string get_action_short();

    /// Set reason of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setReason(libdnf::TransactionItemReason value)
    LIBDNF_LOCAL void set_reason(Reason value);

    /// Set transaction item state
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setState(libdnf::TransactionItemState value)
    LIBDNF_LOCAL void set_state(State value);

    /// Get transaction item repoid (text identifier of a repository)
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setRepoid(const std::string & value)
    LIBDNF_LOCAL void set_repoid(const std::string & value);

    /// Has the item appeared on the system during the transaction?
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.isForwardAction()
    LIBDNF_LOCAL bool is_inbound_action() const;

    /// Has the item got removed from the system during the transaction?
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.isBackwardAction()
    LIBDNF_LOCAL bool is_outbound_action() const;

    // TODO(dmach): Reimplement in Package class; it's most likely not needed in Comps{Group,Environment}
    // std::vector< TransactionItemPtr > replacedBy;
    LIBDNF_LOCAL const Transaction & get_transaction() const;

    // TODO(dmach): Reimplement in Package class
    //const std::vector< TransactionItemPtr > &getReplacedBy() const noexcept { return replacedBy; }
    //void addReplacedBy(TransactionItemPtr value) { if (value) replacedBy.push_back(value); }
    //void saveReplacedBy();

    // TODO(dmach): Review and bring back if needed
    //void saveState();

    /// Get database id (primary key) of the item (table 'item'; other item tables such 'rpm' inherit from it via 1:1 relation)
    LIBDNF_LOCAL int64_t get_item_id() const noexcept;

    /// Set database id (primary key) of the item (table 'item'; other item tables such 'rpm' inherit from it via 1:1 relation)
    LIBDNF_LOCAL void set_item_id(int64_t value);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_ITEM_HPP
