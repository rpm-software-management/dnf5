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


#ifndef LIBDNF5_BASE_TRANSACTION_MODULE_HPP
#define LIBDNF5_BASE_TRANSACTION_MODULE_HPP

#include "libdnf5/base/transaction.hpp"
#include "libdnf5/transaction/transaction_item_action.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"
#include "libdnf5/transaction/transaction_item_state.hpp"


namespace libdnf5::base {

class TransactionModule {
public:
    using Reason = transaction::TransactionItemReason;
    using State = transaction::TransactionItemState;
    using Action = transaction::TransactionItemAction;

    /// @return the module name.
    std::string get_module_name() const;

    /// @return the module stream.
    std::string get_module_stream() const;

    /// @return the action being performed on the transaction module.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getAction()
    Action get_action() const noexcept;

    /// @return the state of the module item in the transaction.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getState()
    State get_state() const noexcept;

    /// @return the reason of the action being performed on the transaction module.
    //
    // @replaces libdnf:transaction/TransactionItem.hpp:method:TransactionItemBase.getReason()
    Reason get_reason() const noexcept;

    ~TransactionModule();

    TransactionModule(const TransactionModule & mpkg);
    TransactionModule & operator=(const TransactionModule & mpkg);
    TransactionModule(TransactionModule && mpkg) noexcept;
    TransactionModule & operator=(TransactionModule && mpkg) noexcept;

private:
    friend class Transaction::Impl;

    TransactionModule(const std::string & module_name, const std::string & module_stream, Action action, Reason reason);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_MODULE_HPP
