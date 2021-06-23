/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_BASE_TRANSACTION_PACKAGE_HPP
#define LIBDNF_BASE_TRANSACTION_PACKAGE_HPP

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/transaction/transaction_item_action.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"
#include "libdnf/transaction/transaction_item_state.hpp"

#include <optional>


namespace libdnf::base {

class TransactionPackage {
public:
    using Action = transaction::TransactionItemAction;
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;

    /// @return the underlying package.
    libdnf::rpm::Package get_package() const { return package; }

    /// @return the action being preformed on the transaction package.
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

    /// Set the reason of the action being performed on the transaction package.
    ///
    /// @param reason The reason to set.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.setReason(libdnf::TransactionItemReason value)
    void set_reason(Reason reason) { this->reason = reason; }

    /// @return the package replaced by this transaction package or nullptr when nothing is replaced.
    const std::optional<rpm::Package> get_replaces() const noexcept { return replaces; }

    /// @return packages obsoleted by this transaction package.
    const std::vector<rpm::Package> & get_obsoletes() const noexcept { return obsoletes; }

    /// @return packages that replace this transaction package (for transaction
    /// packages that are leaving the system).
    const std::vector<rpm::Package> & get_replaced_by() const noexcept { return replaced_by; }

public:
    friend class Transaction;

    TransactionPackage(const libdnf::rpm::Package & pkg, Action action, Reason reason)
        : package(pkg),
          action(action),
          reason(reason) {}

    void set_state(State value) noexcept { state = value; }

    libdnf::rpm::Package package;
    Action action;
    Reason reason;
    State state{State::UNKNOWN};

    std::vector<rpm::Package> obsoletes;
    std::optional<rpm::Package> replaces;
    std::vector<rpm::Package> replaced_by;
};

}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_PACKAGE_HPP
