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


#ifndef LIBDNF5_BASE_TRANSACTION_PACKAGE_HPP
#define LIBDNF5_BASE_TRANSACTION_PACKAGE_HPP

#include "libdnf5/base/transaction.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/rpm/package.hpp"
#include "libdnf5/transaction/transaction_item_action.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"
#include "libdnf5/transaction/transaction_item_state.hpp"

#include <optional>

class BaseGoalTest;
class RpmTransactionTest;

namespace libdnf5::base {

/// Describe transaction operation related to rpm Package
class LIBDNF_API TransactionPackage {
public:
    using Action = transaction::TransactionItemAction;
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;

    /// @return the underlying package.
    libdnf5::rpm::Package get_package() const;

    /// @return the action being performed on the transaction package.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept;

    /// @return the state of the package in the transaction.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept;

    /// @return the reason of the action being performed on the transaction package.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept;

    /// @return packages replaced by this transaction package.
    std::vector<rpm::Package> get_replaces() const noexcept;

    /// @return packages that replace this transaction package (for transaction
    /// packages that are leaving the system).
    const std::vector<rpm::Package> & get_replaced_by() const noexcept;

    /// The REASON_CHANGE action requires group id in case the reason is changed to GROUP
    /// @return id of group the package belongs to
    const std::string * get_reason_change_group_id() const noexcept;

    ~TransactionPackage();

    TransactionPackage(const TransactionPackage & mpkg);
    TransactionPackage & operator=(const TransactionPackage & mpkg);
    TransactionPackage(TransactionPackage && mpkg) noexcept;
    TransactionPackage & operator=(TransactionPackage && mpkg) noexcept;

private:
    friend class Transaction::Impl;
    friend class ::BaseGoalTest;
    friend class ::RpmTransactionTest;

    LIBDNF_LOCAL TransactionPackage(const libdnf5::rpm::Package & pkg, Action action, Reason reason);

    LIBDNF_LOCAL TransactionPackage(const libdnf5::rpm::Package & pkg, Action action, Reason reason, State state);

    LIBDNF_LOCAL TransactionPackage(
        const libdnf5::rpm::Package & pkg, Action action, Reason reason, const std::optional<std::string> & group_id);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_PACKAGE_HPP
