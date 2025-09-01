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

#include "libdnf5/transaction/transaction_item_action.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::transaction {

InvalidTransactionItemAction::InvalidTransactionItemAction(const std::string & action)
    : libdnf5::Error(M_("Invalid transaction item action: {}"), action) {}


std::string transaction_item_action_to_string(TransactionItemAction action) {
    switch (action) {
        case TransactionItemAction::INSTALL:
            return "Install";
        case TransactionItemAction::UPGRADE:
            return "Upgrade";
        case TransactionItemAction::DOWNGRADE:
            return "Downgrade";
        case TransactionItemAction::REINSTALL:
            return "Reinstall";
        case TransactionItemAction::REMOVE:
            return "Remove";
        case TransactionItemAction::REPLACED:
            return "Replaced";
        case TransactionItemAction::REASON_CHANGE:
            return "Reason Change";
        case TransactionItemAction::ENABLE:
            return "Enable";
        case TransactionItemAction::DISABLE:
            return "Disable";
        case TransactionItemAction::RESET:
            return "Reset";
        case TransactionItemAction::SWITCH:
            return "Switch";
    }
    return "";
}


TransactionItemAction transaction_item_action_from_string(const std::string & action) {
    if (action == "Install") {
        return TransactionItemAction::INSTALL;
    } else if (action == "Upgrade") {
        return TransactionItemAction::UPGRADE;
    } else if (action == "Downgrade") {
        return TransactionItemAction::DOWNGRADE;
    } else if (action == "Reinstall") {
        return TransactionItemAction::REINSTALL;
    } else if (action == "Remove") {
        return TransactionItemAction::REMOVE;
    } else if (action == "Replaced") {
        return TransactionItemAction::REPLACED;
    } else if (action == "Reason Change") {
        return TransactionItemAction::REASON_CHANGE;
    } else if (action == "Enable") {
        return TransactionItemAction::ENABLE;
    } else if (action == "Disable") {
        return TransactionItemAction::DISABLE;
    } else if (action == "Reset") {
        return TransactionItemAction::RESET;
    } else if (action == "Switch") {
        return TransactionItemAction::SWITCH;
    }

    throw InvalidTransactionItemAction(action);
}


std::string transaction_item_action_to_letter(TransactionItemAction action) {
    // TODO(dmach): consider adding the direction, e.g. ">U" == "Upgrade", "<U" == "Upgraded"
    switch (action) {
        case TransactionItemAction::INSTALL:
            return "I";
        case TransactionItemAction::UPGRADE:
            return "U";
        case TransactionItemAction::DOWNGRADE:
            return "D";
        case TransactionItemAction::REINSTALL:
            return "R";
        case TransactionItemAction::REMOVE:
            // "R" is for Reinstall, therefore use "E" for rEmove (or Erase)
            return "E";
        case TransactionItemAction::REPLACED:
            return "O";  // TODO(lukash) historically Obsolete, do we change this?
        case TransactionItemAction::REASON_CHANGE:
            // TODO(dmach): replace "?" with something better
            return "?";
        case TransactionItemAction::ENABLE:
        case TransactionItemAction::DISABLE:
        case TransactionItemAction::RESET:
        case TransactionItemAction::SWITCH:
            // TODO(pkratoch): Add letters for ENABLE, DISABLE and RESET
            return "";
    }
    return "";
}


// TODO(pkratoch): Decide inbound/outbound for ENABLE, DISABLE and RESET

bool transaction_item_action_is_inbound(TransactionItemAction action) {
    switch (action) {
        case TransactionItemAction::INSTALL:
        case TransactionItemAction::UPGRADE:
        case TransactionItemAction::DOWNGRADE:
        case TransactionItemAction::REINSTALL:
            return true;
        default:
            return false;
    }
    return false;
}


bool transaction_item_action_is_outbound(TransactionItemAction action) {
    switch (action) {
        case TransactionItemAction::REMOVE:
        case TransactionItemAction::REPLACED:
            return true;
        default:
            return false;
    }
    return false;
}


}  // namespace libdnf5::transaction
