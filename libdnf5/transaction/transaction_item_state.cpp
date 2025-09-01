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
