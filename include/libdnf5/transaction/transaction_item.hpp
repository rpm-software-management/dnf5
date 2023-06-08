/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_TRANSACTION_TRANSACTION_ITEM_HPP
#define LIBDNF_TRANSACTION_TRANSACTION_ITEM_HPP

#include "transaction_item_action.hpp"
#include "transaction_item_reason.hpp"
#include "transaction_item_state.hpp"

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


class TransactionItem {
public:
    using Action = TransactionItemAction;
    using Reason = TransactionItemReason;
    using State = TransactionItemState;

    /// Get action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept { return action; }

    /// Get reason of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept { return reason; }

    /// Get transaction item repoid (text identifier of a repository)
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getRepoid()
    const std::string & get_repoid() const noexcept { return repoid; }

    /// Get transaction item state
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept { return state; }

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

    explicit TransactionItem(const Transaction & trans);

    /// Get database id (primary key) of the transaction item (table 'trans_item')
    int64_t get_id() const noexcept { return id; }

    /// Set database id (primary key) of the transaction item (table 'trans_item')
    void set_id(int64_t value) { id = value; }

    /// Set action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setAction(libdnf::TransactionItemAction value)
    void set_action(Action value) { action = value; }

    /// Get name of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getActionName()
    std::string get_action_name();

    /// Get abbreviated name of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getActionShort()
    std::string get_action_short();

    /// Set reason of the action associated with the transaction item in the transaction
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setReason(libdnf::TransactionItemReason value)
    void set_reason(Reason value) { reason = value; }

    /// Set transaction item state
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setState(libdnf::TransactionItemState value)
    void set_state(State value) { state = value; }

    /// Get transaction item repoid (text identifier of a repository)
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setRepoid(const std::string & value)
    void set_repoid(const std::string & value) { repoid = value; }

    /// Has the item appeared on the system during the transaction?
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.isForwardAction()
    bool is_inbound_action() const;

    /// Has the item got removed from the system during the transaction?
    ///
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.isBackwardAction()
    bool is_outbound_action() const;

    // TODO(dmach): Reimplement in Package class; it's most likely not needed in Comps{Group,Environment}
    // std::vector< TransactionItemPtr > replacedBy;
    const Transaction & get_transaction() const;

    // TODO(dmach): Reimplement in Package class
    //const std::vector< TransactionItemPtr > &getReplacedBy() const noexcept { return replacedBy; }
    //void addReplacedBy(TransactionItemPtr value) { if (value) replacedBy.push_back(value); }
    //void saveReplacedBy();

    // TODO(dmach): Review and bring back if needed
    //void saveState();

    // TODO(dmach): move to sack, resolve for all packages; return the user who initially installed the package
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItem.getInstalledBy()
    uint32_t getInstalledBy() const;

    /// Get database id (primary key) of the item (table 'item'; other item tables such 'rpm' inherit from it via 1:1 relation)
    int64_t get_item_id() const noexcept { return item_id; }

    /// Set database id (primary key) of the item (table 'item'; other item tables such 'rpm' inherit from it via 1:1 relation)
    void set_item_id(int64_t value) { item_id = value; }
    int64_t id = 0;
    Action action = Action::INSTALL;
    Reason reason = Reason::NONE;
    State state = State::STARTED;
    std::string repoid;

    int64_t item_id = 0;

    // TODO(lukash) this won't be safe in bindings (or in general when a
    // TransactionItem is kept around after a Transaction is destroyed), but we
    // can't easily use a WeakPtr here, since the Transactions are expected to
    // be at least movable, and the WeakPtrGuard would make the Transaction
    // unmovable
    const Transaction * trans = nullptr;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF_TRANSACTION_TRANSACTION_ITEM_HPP
