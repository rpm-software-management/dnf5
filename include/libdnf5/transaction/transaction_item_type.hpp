// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TRANSACTION_ITEM_TYPE_HPP
#define LIBDNF5_TRANSACTION_ITEM_TYPE_HPP

#include "transaction_item_errors.hpp"

#include "libdnf5/defs.h"

#include <string>


namespace libdnf5::transaction {

enum class TransactionItemType : int { PACKAGE, GROUP, ENVIRONMENT, MODULE };


LIBDNF_API std::string transaction_item_type_to_string(TransactionItemType action);
LIBDNF_API TransactionItemType transaction_item_type_from_string(const std::string & action);

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_ITEM_TYPE_HPP
