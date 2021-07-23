/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
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

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/query.hpp"
#include "libdnf/common/weak_ptr.hpp"

#include <mutex>
#include <vector>


namespace libdnf::transaction {

class TransactionSack;

using TransactionSackWeakPtr = libdnf::WeakPtr<TransactionSack, false>;

/// Weak pointer to Transaction. TransactionWeakPtr does not own it (ptr_owner = false).
/// Transaction objects are owned by TransactionSack.
using TransactionWeakPtr = libdnf::WeakPtr<Transaction, false>;

class TransactionQuery : public libdnf::sack::Query<TransactionWeakPtr> {
public:
    using libdnf::sack::Query<TransactionWeakPtr>::Query;

    // create an *empty* query
    // the content is lazily loaded/cached while running the queries
    explicit TransactionQuery(const libdnf::BaseWeakPtr & base);
    explicit TransactionQuery(libdnf::Base & base);

    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.dbSelect(int64_t transaction_id)
    TransactionQuery & filter_id(int64_t pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    TransactionQuery & filter_id(const std::vector<int64_t> & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);

private:
    explicit TransactionQuery(const TransactionSackWeakPtr & sack);

    friend TransactionSack;
    TransactionSackWeakPtr sack;

    // Load Transaction objects during first filter call.
    // The following filter calls only modify the previously created set of Transactions.
    bool initialized = false;

    struct F {
        static int64_t id(const TransactionWeakPtr & obj) { return obj->get_id(); }
    };
};

}  // namespace libdnf::transaction

#endif  // LIBDNF_TRANSACTION_QUERY_HPP
