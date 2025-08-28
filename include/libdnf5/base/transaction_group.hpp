// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_BASE_TRANSACTION_GROUP_HPP
#define LIBDNF5_BASE_TRANSACTION_GROUP_HPP

#include "libdnf5/base/transaction.hpp"
#include "libdnf5/comps/group/group.hpp"
#include "libdnf5/comps/group/package.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/rpm/package.hpp"
#include "libdnf5/transaction/transaction_item_action.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"
#include "libdnf5/transaction/transaction_item_state.hpp"


namespace libdnf5::base {

class LIBDNF_API TransactionGroup {
public:
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;
    using Action = transaction::TransactionItemAction;
    using PackageType = libdnf5::comps::PackageType;

    /// @return the underlying group.
    libdnf5::comps::Group get_group() const;

    /// @return the action being performed on the transaction group.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept;

    /// @return the state of the group in the transaction.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept;

    /// @return the reason of the action being performed on the transaction group.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept;

    /// @return package types requested to be installed with the group.
    PackageType get_package_types() const noexcept;

    ~TransactionGroup();

    TransactionGroup(const TransactionGroup & mpkg);
    TransactionGroup & operator=(const TransactionGroup & mpkg);
    TransactionGroup(TransactionGroup && mpkg) noexcept;
    TransactionGroup & operator=(TransactionGroup && mpkg) noexcept;

private:
    friend class Transaction::Impl;

    LIBDNF_LOCAL TransactionGroup(
        const libdnf5::comps::Group & grp, Action action, Reason reason, const PackageType & types);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_GROUP_HPP
