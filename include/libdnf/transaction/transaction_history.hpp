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

#ifndef LIBDNF_TRANSACTION_TRANSACTION_HISTORY_HPP
#define LIBDNF_TRANSACTION_TRANSACTION_HISTORY_HPP

#include "transaction.hpp"

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/weak_ptr.hpp"


namespace libdnf::transaction {

class TransactionHistory;
using TransactionHistoryWeakPtr = libdnf::WeakPtr<TransactionHistory, false>;

/// A class for working with transactions recorded in the transaction history database.
class TransactionHistory {
public:
    explicit TransactionHistory(const libdnf::BaseWeakPtr & base);
    explicit TransactionHistory(libdnf::Base & base);

    /// Create a new Transaction object.
    // TODO(lukash) make private -> move out of the public interface
    libdnf::transaction::Transaction new_transaction();

    TransactionHistoryWeakPtr get_weak_ptr() { return TransactionHistoryWeakPtr(this, &guard); }

    /// Lists transactions from the transaction history for transaction ids in `ids`.
    ///
    /// @param ids The ids to list.
    /// @return The listed transactions.
    std::vector<Transaction> list_transactions(const std::vector<int64_t> & ids);

    /// Lists transactions from the transaction history for transaction ids
    /// within the [start, end] range (inclusive).
    ///
    /// @param start The first id of the range to be listed.
    /// @param end The last id of the range to be listed.
    /// @return The listed transactions.
    std::vector<Transaction> list_transactions(int64_t start, int64_t end);

    /// Lists all transactions from the transaction history.
    ///
    /// @return The listed transactions.
    std::vector<Transaction> list_all_transactions();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;

private:
    BaseWeakPtr base;

    WeakPtrGuard<TransactionHistory, false> guard;
};

}  // namespace libdnf::transaction

#endif  // LIBDNF_TRANSACTION_TRANSACTION_HISTORY_HPP
