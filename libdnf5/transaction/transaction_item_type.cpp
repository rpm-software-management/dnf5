// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
