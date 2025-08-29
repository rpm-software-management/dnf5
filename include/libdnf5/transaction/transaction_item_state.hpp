// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_ITEM_STATE_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_ITEM_STATE_HPP

#include "transaction_item_errors.hpp"

#include "libdnf5/defs.h"

#include <string>


namespace libdnf5::transaction {

enum class TransactionItemState : int { STARTED = 1, OK = 2, ERROR = 3 };


LIBDNF_API std::string transaction_item_state_to_string(TransactionItemState state);
LIBDNF_API TransactionItemState transaction_item_state_from_string(const std::string & state);

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_ITEM_STATE_HPP
