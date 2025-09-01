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

// TODO(dmach): keep refactoring and deliver something that works with the new code base
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#ifndef LIBDNF5_TRANSACTION_MERGEDTRANSACTION_HPP
#define LIBDNF5_TRANSACTION_MERGEDTRANSACTION_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace libdnf5::transaction {

class MergedTransaction;
typedef std::shared_ptr<MergedTransaction> MergedTransactionPtr;
}  // namespace libdnf5::transaction

#include "rpm_package.hpp"
#include "transaction.hpp"
#include "transaction_item.hpp"

namespace libdnf5::transaction {

class MergedTransaction {
public:
    explicit MergedTransaction(Transaction & trans);
    void merge(Transaction & trans);

    std::vector<int64_t> listIds() const;
    std::vector<uint32_t> listUserIds() const;
    std::vector<std::string> listCmdlines() const;
    std::vector<TransactionState> listStates() const;
    std::vector<std::string> listReleasevers() const;
    int64_t get_dt_begin() const noexcept;
    int64_t get_dt_end() const noexcept;
    const std::string & get_rpmdb_version_begin() const noexcept;
    const std::string & get_rpmdb_version_end() const noexcept;
    std::set<std::string> get_runtime_packages() const;
    std::vector<std::pair<int, std::string>> get_console_output();

    std::vector<std::unique_ptr<CompsEnvironment>> get_comps_environments();
    std::vector<std::unique_ptr<CompsGroup>> get_comps_groups();
    std::vector<std::unique_ptr<Package>> get_packages();

protected:
    std::vector<Transaction *> transactions;

    class ItemPair {
    public:
        ItemPair(TransactionItem * first, TransactionItem * second) : first{first}, second{second} {}
        //ItemPair(){};
        TransactionItem * first = nullptr;
        TransactionItem * second = nullptr;
    };

    using ItemPairMap = std::map<std::string, ItemPair>;

    void mergeItem(ItemPairMap & itemPairMap, TransactionItem * transItem);
    void resolveRPMDifference(ItemPair & previousItemPair, TransactionItem * mTransItem);
    void resolveErase(ItemPair & previousItemPair, TransactionItem * mTransItem);
    void resolveAltered(ItemPair & previousItemPair, TransactionItem * mTransItem);
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_HPP

#endif
