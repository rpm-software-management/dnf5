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


#ifndef LIBDNF5_BASE_TRANSACTION_GROUP_HPP
#define LIBDNF5_BASE_TRANSACTION_GROUP_HPP

#include "libdnf5/base/goal_elements.hpp"
#include "libdnf5/base/transaction.hpp"
#include "libdnf5/comps/group/group.hpp"
#include "libdnf5/comps/group/package.hpp"
#include "libdnf5/rpm/package.hpp"
#include "libdnf5/transaction/transaction_item_action.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"
#include "libdnf5/transaction/transaction_item_state.hpp"

#include <optional>


namespace libdnf5::base {

class TransactionGroup {
public:
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;
    using Action = transaction::TransactionItemAction;
    using PackageType = libdnf5::comps::PackageType;

    /// @return the underlying group.
    libdnf5::comps::Group get_group() const { return group; }

    /// @return the action being performed on the transaction group.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept { return action; }

    /// @return the state of the group in the transaction.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept { return state; }

    /// @return the reason of the action being performed on the transaction group.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept { return reason; }

    /// @return package types requested to be installed with the group.
    PackageType get_package_types() const noexcept { return package_types; }

private:
    friend class Transaction::Impl;

    TransactionGroup(const libdnf5::comps::Group & grp, Action action, Reason reason, const PackageType types)
        : group(grp),
          action(action),
          reason(reason),
          package_types(types) {}

    void set_state(State value) noexcept { state = value; }

    libdnf5::comps::Group group;
    Action action;
    Reason reason;
    State state{State::STARTED};
    PackageType package_types;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_GROUP_HPP
