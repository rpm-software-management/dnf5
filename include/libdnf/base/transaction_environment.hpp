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


#ifndef LIBDNF_BASE_TRANSACTION_ENVIRONMENT_HPP
#define LIBDNF_BASE_TRANSACTION_ENVIRONMENT_HPP

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/transaction.hpp"
#include "libdnf/comps/environment/environment.hpp"
#include "libdnf/transaction/transaction_item_action.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"
#include "libdnf/transaction/transaction_item_state.hpp"


namespace libdnf5::base {

class TransactionEnvironment {
public:
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;
    using Action = transaction::TransactionItemAction;
    using PackageType = libdnf5::comps::PackageType;

    /// @return the underlying environment.
    libdnf5::comps::Environment get_environment() const { return environment; }

    /// @return the action being preformed on the transaction environment.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept { return action; }

    /// @return the state of the environment in the transaction.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept { return state; }

    /// @return the reason of the action being performed on the transaction environment.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept { return reason; }

    /// @return package types requested to be installed with the group.
    bool get_with_optional() const noexcept { return with_optional; }

private:
    friend class Transaction::Impl;

    TransactionEnvironment(
        const libdnf5::comps::Environment & grp, Action action, Reason reason, const bool with_optional)
        : environment(grp),
          action(action),
          reason(reason),
          with_optional(with_optional) {}

    void set_state(State value) noexcept { state = value; }

    libdnf5::comps::Environment environment;
    Action action;
    Reason reason;
    State state{State::STARTED};
    bool with_optional;
};

}  // namespace libdnf5::base

#endif  // LIBDNF_BASE_TRANSACTION_ENVIRONMENT_HPP
