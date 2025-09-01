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
