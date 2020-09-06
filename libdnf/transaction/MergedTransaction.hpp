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

namespace libdnf {

class MergedTransaction;
typedef std::shared_ptr< MergedTransaction > MergedTransactionPtr;
}

#include "RPMItem.hpp"
#include "Transaction.hpp"
#include "TransactionItem.hpp"

namespace libdnf {

class MergedTransaction {
public:
    explicit MergedTransaction(TransactionPtr trans);
    void merge(TransactionPtr trans);

    std::vector< int64_t > listIds() const;
    std::vector< uint32_t > listUserIds() const;
    std::vector< std::string > listCmdlines() const;
    std::vector< TransactionState > listStates() const;
    std::vector< std::string > listReleasevers() const;
    int64_t getDtBegin() const noexcept;
    int64_t getDtEnd() const noexcept;
    const std::string &getRpmdbVersionBegin() const noexcept;
    const std::string &getRpmdbVersionEnd() const noexcept;
    std::set< RPMItemPtr > getSoftwarePerformedWith() const;
    std::vector< std::pair< int, std::string > > getConsoleOutput();

    std::vector< TransactionItemBasePtr > getItems();

protected:
    std::vector< TransactionPtr > transactions;

    struct ItemPair {
        ItemPair(TransactionItemBasePtr first, TransactionItemBasePtr second)
          : first{first}
          , second{second}
        {
        }
        ItemPair(){};
        TransactionItemBasePtr first = nullptr;
        TransactionItemBasePtr second = nullptr;
    };

    typedef std::map< std::string, ItemPair > ItemPairMap;

    void mergeItem(ItemPairMap &itemPairMap, TransactionItemBasePtr transItem);
    void resolveRPMDifference(ItemPair &previousItemPair, TransactionItemBasePtr mTransItem);
    void resolveErase(ItemPair &previousItemPair, TransactionItemBasePtr mTransItem);
    void resolveAltered(ItemPair &previousItemPair, TransactionItemBasePtr mTransItem);
};

} // namespace libdnf

#endif // LIBDNF_TRANSACTION_TRANSACTION_HPP
