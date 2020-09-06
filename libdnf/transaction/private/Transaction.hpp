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

#ifndef LIBDNF_TRANSACTION_TRANSACTION_PRIVATE_HPP
#define LIBDNF_TRANSACTION_TRANSACTION_PRIVATE_HPP

#include "../Transaction.hpp"

namespace libdnf {
namespace swdb_private {

class Transaction;
typedef std::shared_ptr< Transaction > TransactionPtr;

class Transaction : public libdnf::Transaction {
public:
    // create an empty object, don't read from db
    explicit Transaction(SQLite3Ptr conn);

    void setId(int64_t value) { id = value; }
    void setDtBegin(int64_t value) { dtBegin = value; }
    void setDtEnd(int64_t value) { dtEnd = value; }
    void setRpmdbVersionBegin(const std::string &value) { rpmdbVersionBegin = value; }
    void setRpmdbVersionEnd(const std::string &value) { rpmdbVersionEnd = value; }
    void setReleasever(const std::string &value) { releasever = value; }
    void setUserId(uint32_t value) { userId = value; }
    void setCmdline(const std::string &value) { cmdline = value; }
    void setState(TransactionState value) { state = value; }

    std::vector< TransactionItemPtr > getItems() override;

    void begin();
    void finish(TransactionState state);
    TransactionItemPtr addItem(std::shared_ptr< Item > item,
                               const std::string &repoid,
                               TransactionItemAction action,
                               TransactionItemReason reason);

    void addConsoleOutputLine(int fileDescriptor, const std::string &line);
    void addSoftwarePerformedWith(std::shared_ptr< RPMItem > software);

protected:
    void saveItems();
    std::vector< TransactionItemPtr > items;

    void dbInsert();
    void dbUpdate();
};

}
}

#endif // LIBDNF_TRANSACTION_TRANSACTION_PRIVATE_HPP
