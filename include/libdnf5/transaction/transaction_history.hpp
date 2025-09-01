// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_HISTORY_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_HISTORY_HPP

#include "transaction.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/defs.h"


namespace libdnf5::transaction {

class TransactionHistory;
using TransactionHistoryWeakPtr = libdnf5::WeakPtr<TransactionHistory, false>;

/// A class for working with transactions recorded in the transaction history database.
class LIBDNF_API TransactionHistory {
public:
    explicit TransactionHistory(const libdnf5::BaseWeakPtr & base);
    explicit TransactionHistory(libdnf5::Base & base);
    ~TransactionHistory();
    TransactionHistory(const TransactionHistory & src) = delete;
    TransactionHistory & operator=(const TransactionHistory & src) = delete;
    TransactionHistory(TransactionHistory && src) noexcept = delete;
    TransactionHistory & operator=(TransactionHistory && src) noexcept = delete;

    TransactionHistoryWeakPtr get_weak_ptr();

    /// Lists all transaction IDs from the transaction history database. The
    /// result is sorted in ascending order.
    ///
    /// @return The list of transaction IDs.
    std::vector<int64_t> list_transaction_ids();

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
    libdnf5::BaseWeakPtr get_base() const;

    /// Get reason for package specified by name and arch at a point in history
    /// specified by transaction id.
    ///
    /// @param name Name of rpm package
    /// @param arch Arch of rpm package
    /// @param transaction_id_point Id of a history transaction (can be obtained from
    ///                             libdnf5::transaction::TransactionHistory)
    /// @return Reason of the last transaction item before transaction_id_point that
    ///         has an rpm with matching name and arch.
    TransactionItemReason transaction_item_reason_at(
        const std::string & name, const std::string & arch, int64_t transaction_id_point);

    /// Get counts of transaction items for specified transactions.
    /// It gets the counts in a single db query.
    ///
    /// @param transactions Get counts for these transactions.
    ///
    /// @return Mapped transaction id -> count.
    std::unordered_map<int64_t, int64_t> get_transaction_item_counts(const std::vector<Transaction> & transactions);

    /// Filter out transactions that don't contain any rpm with matching name
    ///
    /// @param transactions     Vector of Transactions to filter
    /// @param pkg_names        Vector of rpm package names to match
    void filter_transactions_by_pkg_names(
        std::vector<Transaction> & transactions, const std::vector<std::string> & pkg_names);

private:
    /// Create a new Transaction object.
    LIBDNF_LOCAL libdnf5::transaction::Transaction new_transaction();

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_HISTORY_HPP
