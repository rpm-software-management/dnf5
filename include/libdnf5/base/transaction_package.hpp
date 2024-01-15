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


#ifndef LIBDNF5_BASE_TRANSACTION_PACKAGE_HPP
#define LIBDNF5_BASE_TRANSACTION_PACKAGE_HPP

#include "libdnf5/base/goal_elements.hpp"
#include "libdnf5/base/transaction.hpp"
#include "libdnf5/rpm/package.hpp"
#include "libdnf5/transaction/transaction_item_action.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"
#include "libdnf5/transaction/transaction_item_state.hpp"

#include <optional>

class BaseGoalTest;
class RpmTransactionTest;

namespace libdnf5::base {

/// Describe transaction operation related to rpm Package
class TransactionPackage {
public:
    using Action = transaction::TransactionItemAction;
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;

    /// @return the underlying package.
    libdnf5::rpm::Package get_package() const { return package; }

    /// @return the action being performed on the transaction package.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept { return action; }

    /// @return the state of the package in the transaction.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept { return state; }

    /// @return the reason of the action being performed on the transaction package.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept { return reason; }

    /// @return packages replaced by this transaction package.
    const std::vector<rpm::Package> get_replaces() const noexcept { return replaces; }

    /// @return packages that replace this transaction package (for transaction
    /// packages that are leaving the system).
    const std::vector<rpm::Package> & get_replaced_by() const noexcept { return replaced_by; }

    /// The REASON_CHANGE action requires group id in case the reason is changed to GROUP
    /// @return id of group the package belongs to
    const std::string * get_reason_change_group_id() const noexcept {
        return reason_change_group_id ? &reason_change_group_id.value() : nullptr;
    }

private:
    friend class Transaction::Impl;
    friend class ::BaseGoalTest;
    friend class ::RpmTransactionTest;

    TransactionPackage(const libdnf5::rpm::Package & pkg, Action action, Reason reason)
        : package(pkg),
          action(action),
          reason(reason) {}

    TransactionPackage(const libdnf5::rpm::Package & pkg, Action action, Reason reason, State state)
        : package(pkg),
          action(action),
          reason(reason),
          state(state) {}

    TransactionPackage(
        const libdnf5::rpm::Package & pkg, Action action, Reason reason, std::optional<std::string> group_id)
        : package(pkg),
          action(action),
          reason(reason),
          reason_change_group_id(group_id) {}

    void set_reason(Reason value) noexcept { reason = value; }
    void set_state(State value) noexcept { state = value; }

    libdnf5::rpm::Package package;
    Action action;
    Reason reason;
    State state{State::STARTED};
    std::optional<std::string> reason_change_group_id;

    std::vector<rpm::Package> replaces;
    std::vector<rpm::Package> replaced_by;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_PACKAGE_HPP
