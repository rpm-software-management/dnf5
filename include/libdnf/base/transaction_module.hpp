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


#ifndef LIBDNF_BASE_TRANSACTION_MODULE_HPP
#define LIBDNF_BASE_TRANSACTION_MODULE_HPP

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/transaction.hpp"
#include "libdnf/module/module_item.hpp"
#include "libdnf/transaction/transaction_item_action.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"
#include "libdnf/transaction/transaction_item_state.hpp"

#include <optional>


namespace libdnf::base {

class TransactionModule {
public:
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;
    using Action = transaction::TransactionItemAction;

    /// @return the module name.
    std::string get_module_name() const { return module_name; }

    /// @return the module stream.
    std::string get_module_stream() const { return module_stream; }

    /// @return the action being preformed on the transaction module.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept { return action; }

    /// @return the state of the module item in the transaction.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept { return state; }

    /// @return the reason of the action being performed on the transaction module.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept { return reason; }

private:
    friend class Transaction::Impl;

    TransactionModule(const std::string & module_name, const std::string & module_stream, Action action, Reason reason)
        : module_name(module_name),
          module_stream(module_stream),
          action(action),
          reason(reason) {}

    void set_state(State value) noexcept { state = value; }

    std::string module_name;
    std::string module_stream;
    Action action;
    Reason reason;
    State state{State::STARTED};
};

}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_MODULE_HPP
