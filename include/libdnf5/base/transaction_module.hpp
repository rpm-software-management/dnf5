// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_BASE_TRANSACTION_MODULE_HPP
#define LIBDNF5_BASE_TRANSACTION_MODULE_HPP

#include "libdnf5/base/transaction.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/transaction/transaction_item_action.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"
#include "libdnf5/transaction/transaction_item_state.hpp"


namespace libdnf5::base {

class LIBDNF_API TransactionModule {
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

    /// @return module:stream pairs replaced by this transaction module.
    std::vector<std::pair<std::string, std::string>> get_replaces() const noexcept;

    /// @return module:stream pairs that replace this transaction module (for transaction
    /// modules that are leaving the system).
    const std::vector<std::pair<std::string, std::string>> & get_replaced_by() const noexcept;

    ~TransactionModule();

    TransactionModule(const TransactionModule & mpkg);
    TransactionModule & operator=(const TransactionModule & mpkg);
    TransactionModule(TransactionModule && mpkg) noexcept;
    TransactionModule & operator=(TransactionModule && mpkg) noexcept;

private:
    friend class Transaction::Impl;

    LIBDNF_LOCAL TransactionModule(
        const std::string & module_name, const std::string & module_stream, Action action, Reason reason);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_MODULE_HPP
