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

#ifndef LIBDNF_TRANSACTION_TYPES_HPP
#define LIBDNF_TRANSACTION_TYPES_HPP

#include "transaction_item_action.hpp"
#include "transaction_item_reason.hpp"

namespace libdnf::transaction {


enum class TransactionItemState : int {
    UNKNOWN = 0,  // default state, must be changed before save
    DONE = 1,
    ERROR = 2
};

enum class ItemType : int { UNKNOWN = 0, RPM = 1, GROUP = 2, ENVIRONMENT = 3 };


}  // namespace libdnf::transaction


#endif // LIBDNF_TRANSACTION_TYPES_HPP
