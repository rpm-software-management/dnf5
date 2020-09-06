/*
 * Copyright (C) 2017-2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef LIBDNF_TRANSACTION_TRANSACTIONITEMREASON_HPP
#define LIBDNF_TRANSACTION_TRANSACTIONITEMREASON_HPP


#include <string>


namespace libdnf {

enum class TransactionItemReason : int {
    UNKNOWN = 0,
    DEPENDENCY = 1,
    USER = 2,
    CLEAN = 3, // hawkey compatibility
    WEAK_DEPENDENCY = 4,
    GROUP = 5
};


const std::string &
TransactionItemReasonToString(TransactionItemReason reason);


inline bool operator<(TransactionItemReason lhs, TransactionItemReason rhs)
{
    if (lhs == rhs) {
        return false;
    }
    TransactionItemReason order[] = {
        TransactionItemReason::UNKNOWN,
        TransactionItemReason::CLEAN,
        TransactionItemReason::WEAK_DEPENDENCY,
        TransactionItemReason::DEPENDENCY,
        TransactionItemReason::GROUP,
        TransactionItemReason::USER,
    };
    for (auto i : order) {
        // iterate through 'order' and return according to which value matches
        if (lhs == i) {
            return true;
        }
        if (rhs == i) {
            return false;
        }
    }
    return false;
}

inline bool operator<=(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return true;
    }
    return lhs < rhs;
}


inline bool operator>(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return false;
    }
    return rhs < lhs;
}

inline bool operator>=(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return true;
    }
    return lhs > rhs;
}


inline int
TransactionItemReasonCompare(TransactionItemReason lhs, TransactionItemReason rhs)
{
    if (lhs < rhs) {
        return -1;
    } else if (lhs > rhs) {
        return 1;
    } else {
        return 0;
    }
}


} // namespace libdnf

#endif // LIBDNF_TRANSACTION_TRANSACTIONITEMREASON_HPP
