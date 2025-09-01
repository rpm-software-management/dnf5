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

#include "libdnf5/transaction/transaction_item_type.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::transaction {

InvalidTransactionItemType::InvalidTransactionItemType(const std::string & type)
    : libdnf5::Error(M_("Invalid transaction item type: {}"), type) {}


std::string transaction_item_type_to_string(TransactionItemType type) {
    switch (type) {
        case TransactionItemType::PACKAGE:
            return "Package";
        case TransactionItemType::GROUP:
            return "Group";
        case TransactionItemType::ENVIRONMENT:
            return "Environment";
        case TransactionItemType::MODULE:
            return "Module";
    }
    return "";
}


TransactionItemType transaction_item_type_from_string(const std::string & type) {
    if (type == "Package") {
        return TransactionItemType::PACKAGE;
    } else if (type == "Group") {
        return TransactionItemType::GROUP;
    } else if (type == "Environment") {
        return TransactionItemType::ENVIRONMENT;
    } else if (type == "Module") {
        return TransactionItemType::MODULE;
    }

    throw InvalidTransactionItemType(type);
}


}  // namespace libdnf5::transaction
