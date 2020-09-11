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

#ifndef LIBDNF_TRANSACTION_MERGEDTRANSACTION_HPP
#define LIBDNF_TRANSACTION_MERGEDTRANSACTION_HPP

#include <memory>
#include <set>
#include <string>
#include <map>
#include <vector>

namespace libdnf::transaction {

class MergedTransaction;
typedef std::shared_ptr< MergedTransaction > MergedTransactionPtr;
}

#include "RPMItem.hpp"
#include "transaction.hpp"
#include "transaction_item.hpp"

namespace libdnf::transaction {

class MergedTransaction {
public:
    explicit MergedTransaction(TransactionPtr trans);
    void merge(TransactionPtr trans);

    std::vector< int64_t > listIds() const;
    std::vector< uint32_t > listUserIds() const;
    std::vector< std::string > listCmdlines() const;
    std::vector< TransactionState > listStates() const;
    std::vector< std::string > listReleasevers() const;
    int64_t get_dt_begin() const noexcept;
    int64_t get_dt_end() const noexcept;
    const std::string &get_rpmdb_version_begin() const noexcept;
    const std::string &get_rpmdb_version_end() const noexcept;
    std::set< RPMItemPtr > getSoftwarePerformedWith() const;
    std::vector< std::pair< int, std::string > > getConsoleOutput();

    std::vector< TransactionItemPtr > getItems();

protected:
    std::vector< TransactionPtr > transactions;

    struct ItemPair {
        ItemPair(TransactionItemPtr first, TransactionItemPtr second)
          : first{first}
          , second{second}
        {
        }
        ItemPair(){};
        TransactionItemPtr first = nullptr;
        TransactionItemPtr second = nullptr;
    };

    typedef std::map< std::string, ItemPair > ItemPairMap;

    void mergeItem(ItemPairMap &itemPairMap, TransactionItemPtr transItem);
    void resolveRPMDifference(ItemPair &previousItemPair, TransactionItemPtr mTransItem);
    void resolveErase(ItemPair &previousItemPair, TransactionItemPtr mTransItem);
    void resolveAltered(ItemPair &previousItemPair, TransactionItemPtr mTransItem);
};

}  // namespace libdnf::transaction

#endif // LIBDNF_TRANSACTION_TRANSACTION_HPP
