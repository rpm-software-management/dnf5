// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/transaction/transaction_item_state.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::transaction {

InvalidTransactionItemState::InvalidTransactionItemState(const std::string & state)
    : libdnf5::Error(M_("Invalid transaction item state: {}"), state) {}


std::string transaction_item_state_to_string(TransactionItemState state) {
    switch (state) {
        case TransactionItemState::STARTED:
            return "Started";
        case TransactionItemState::OK:
            return "Ok";
        case TransactionItemState::ERROR:
            return "Error";
    }
    return "";
}


TransactionItemState transaction_item_state_from_string(const std::string & state) {
    if (state == "Started") {
        return TransactionItemState::STARTED;
    } else if (state == "Ok") {
        return TransactionItemState::OK;
    } else if (state == "Error") {
        return TransactionItemState::ERROR;
    }

    throw InvalidTransactionItemState(state);
}


}  // namespace libdnf5::transaction
