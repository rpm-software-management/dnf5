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


#ifndef LIBDNF_TRANSACTION_QUERY_HPP
#define LIBDNF_TRANSACTION_QUERY_HPP


#include "transaction.hpp"

#include "libdnf/common/sack/query.hpp"
#include "libdnf/utils/weak_ptr.hpp"

#include <mutex>
#include <vector>


namespace libdnf::transaction {


class TransactionSack;


/// Weak pointer to Transaction. TransactionWeakPtr does not own it (ptr_owner = false).
/// Transaction objects are owned by TransactionSack.
using TransactionWeakPtr = libdnf::WeakPtr<Transaction, false>;


class TransactionQuery : public libdnf::sack::Query<TransactionWeakPtr> {
public:
    using libdnf::sack::Query<TransactionWeakPtr>::Query;

    TransactionQuery(TransactionSack & sack);

    TransactionQuery & ifilter_id(sack::QueryCmp cmp, int64_t pattern);
    TransactionQuery & ifilter_id(sack::QueryCmp cmp, const std::vector<int64_t> & pattern);

private:
    friend TransactionSack;
    TransactionSack * sack = nullptr;

    // Load Transaction objects during first ifilter call.
    // The following ifilter calls only modify the previously created set of Transactions.
    bool initialized = false;

    struct F {
        static int64_t id(const TransactionWeakPtr & obj) { return obj->get_id(); }
    };
};


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_QUERY_HPP
