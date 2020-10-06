/*
Copyright (C) 2020 Red Hat, Inc.

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


#include "sack.hpp"

#include "libdnf/base/base.hpp"


namespace libdnf::transaction {


TransactionSack::TransactionSack(libdnf::Base & base) : base{base} {}


TransactionSack::~TransactionSack() = default;


TransactionQuery TransactionSack::new_query() {
    // create an *empty* query
    // the content is lazily loaded/cached while running the queries
    TransactionQuery q;
    q.sack = this;
    return q;
}


TransactionWeakPtr TransactionSack::new_transaction() {
    return add_item_with_return(std::make_unique<Transaction>(*this));
}


}  // namespace libdnf::transaction
