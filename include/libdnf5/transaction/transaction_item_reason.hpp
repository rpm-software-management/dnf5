// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_ITEM_REASON_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_ITEM_REASON_HPP

#include "transaction_item_errors.hpp"

#include "libdnf5/defs.h"

#include <string>


namespace libdnf5::transaction {

enum class TransactionItemReason : int {
    NONE = 0,
    DEPENDENCY = 1,
    USER = 2,
    CLEAN = 3,
    WEAK_DEPENDENCY = 4,
    GROUP = 5,
    EXTERNAL_USER = 6
};


LIBDNF_API std::string transaction_item_reason_to_string(TransactionItemReason reason);
LIBDNF_API TransactionItemReason transaction_item_reason_from_string(const std::string & reason);


/// Compare transaction items and return:
/// -1 if lhs < rhs
/// 1 if lhs > rhs
/// 0 if lhs == rhs
/// Higher number means a better (or a stronger) reason.
LIBDNF_API int transaction_item_reason_compare(TransactionItemReason lhs, TransactionItemReason rhs);


LIBDNF_API bool operator<(TransactionItemReason lhs, TransactionItemReason rhs);
LIBDNF_API bool operator<=(TransactionItemReason lhs, TransactionItemReason rhs);
LIBDNF_API bool operator>(TransactionItemReason lhs, TransactionItemReason rhs);
LIBDNF_API bool operator>=(TransactionItemReason lhs, TransactionItemReason rhs);

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_ITEM_REASON_HPP
