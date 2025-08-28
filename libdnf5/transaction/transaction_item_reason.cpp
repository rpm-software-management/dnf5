// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/transaction/transaction_item_reason.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::transaction {

InvalidTransactionItemReason::InvalidTransactionItemReason(const std::string & reason)
    : libdnf5::Error(M_("Invalid transaction item reason: {}"), reason) {}


std::string transaction_item_reason_to_string(TransactionItemReason reason) {
    switch (reason) {
        case TransactionItemReason::NONE:
            return "None";
        case TransactionItemReason::DEPENDENCY:
            return "Dependency";
        case TransactionItemReason::USER:
            return "User";
        case TransactionItemReason::CLEAN:
            return "Clean";
        case TransactionItemReason::WEAK_DEPENDENCY:
            return "Weak Dependency";
        case TransactionItemReason::GROUP:
            return "Group";
        case TransactionItemReason::EXTERNAL_USER:
            return "External User";
    }
    return "";
}


TransactionItemReason transaction_item_reason_from_string(const std::string & reason) {
    if (reason == "None") {
        return TransactionItemReason::NONE;
    } else if (reason == "Dependency") {
        return TransactionItemReason::DEPENDENCY;
    } else if (reason == "User") {
        return TransactionItemReason::USER;
    } else if (reason == "Clean") {
        return TransactionItemReason::CLEAN;
    } else if (reason == "Weak Dependency") {
        return TransactionItemReason::WEAK_DEPENDENCY;
    } else if (reason == "Group") {
        return TransactionItemReason::GROUP;
    } else if (reason == "External User") {
        return TransactionItemReason::EXTERNAL_USER;
    }

    throw InvalidTransactionItemReason(reason);
}


static TransactionItemReason order[] = {
    TransactionItemReason::NONE,
    TransactionItemReason::CLEAN,
    TransactionItemReason::WEAK_DEPENDENCY,
    TransactionItemReason::DEPENDENCY,
    TransactionItemReason::EXTERNAL_USER,
    TransactionItemReason::GROUP,
    TransactionItemReason::USER,
};


bool operator<(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return false;
    }
    for (auto reason : order) {
        // iterate through 'order' and return according to which value matches
        if (lhs == reason) {
            return true;
        }
        if (rhs == reason) {
            return false;
        }
    }
    return false;
}


bool operator<=(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return true;
    }
    return lhs < rhs;
}


bool operator>(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return false;
    }
    return rhs < lhs;
}


bool operator>=(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return true;
    }
    return lhs > rhs;
}


int transaction_item_reason_compare(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs < rhs) {
        return -1;
    }
    if (lhs > rhs) {
        return 1;
    } else {
        return 0;
    }
}


}  // namespace libdnf5::transaction
