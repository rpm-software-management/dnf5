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

#ifndef LIBDNF_TRANSACTION_TRANSACTION_HPP
#define LIBDNF_TRANSACTION_TRANSACTION_HPP

#include <memory>
#include <set>
#include <string>

#include "libdnf/utils/sqlite3/sqlite3.hpp"

namespace libdnf {
class Transaction;
typedef std::shared_ptr< Transaction > TransactionPtr;
}

#include "Item.hpp"
#include "TransactionItem.hpp"

namespace libdnf {

class Transaction {
public:
    // load from db
    Transaction(libdnf::utils::SQLite3Ptr conn, int64_t pk);
    virtual ~Transaction() = default;

    bool operator==(const Transaction &other) const;
    bool operator<(const Transaction &other) const;
    bool operator>(const Transaction &other) const;

    int64_t getId() const noexcept { return id; }
    int64_t getDtBegin() const noexcept { return dtBegin; }
    int64_t getDtEnd() const noexcept { return dtEnd; }
    const std::string &getRpmdbVersionBegin() const noexcept { return rpmdbVersionBegin; }
    const std::string &getRpmdbVersionEnd() const noexcept { return rpmdbVersionEnd; }
    const std::string &getReleasever() const noexcept { return releasever; }
    uint32_t getUserId() const noexcept { return userId; }
    const std::string &getCmdline() const noexcept { return cmdline; }
    TransactionState getState() const noexcept { return state; }

    virtual std::vector< TransactionItemPtr > getItems();
    const std::set< std::shared_ptr< RPMItem > > getSoftwarePerformedWith() const;
    std::vector< std::pair< int, std::string > > getConsoleOutput() const;

protected:
    explicit Transaction(libdnf::utils::SQLite3Ptr conn);
    void dbSelect(int64_t transaction_id);
    std::set< std::shared_ptr< RPMItem > > softwarePerformedWith;

    friend class TransactionItem;
    libdnf::utils::SQLite3Ptr conn;

    int64_t id = 0;
    int64_t dtBegin = 0;
    int64_t dtEnd = 0;
    std::string rpmdbVersionBegin;
    std::string rpmdbVersionEnd;
    // TODO: move to a new "vars" table?
    std::string releasever;
    uint32_t userId = 0;
    std::string cmdline;
    TransactionState state = TransactionState::UNKNOWN;
};

} // namespace libdnf

#endif // LIBDNF_TRANSACTION_TRANSACTION_HPP
