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
