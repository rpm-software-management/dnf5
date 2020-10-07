/*
Copyright (C) 2017-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_TRANSACTION_TRANSACTION_ITEM_STATE_HPP
#define LIBDNF_TRANSACTION_TRANSACTION_ITEM_STATE_HPP


namespace libdnf::transaction {


enum class TransactionItemState : int {
    UNKNOWN = 0,  // default state, must be changed before save
    DONE = 1,
    ERROR = 2
};


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_TRANSACTION_ITEM_STATE_HPP
