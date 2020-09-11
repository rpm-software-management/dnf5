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

#ifndef LIBDNF_TRANSACTION_RPMITEM_HPP
#define LIBDNF_TRANSACTION_RPMITEM_HPP

#include <memory>
#include <vector>

namespace libdnf::transaction {
class RPMItem;
typedef std::shared_ptr< RPMItem > RPMItemPtr;
}

#include "Item.hpp"
#include "TransactionItem.hpp"
#include "Types.hpp"

namespace libdnf::transaction {

class RPMItem : public Item {
public:
    explicit RPMItem(libdnf::utils::SQLite3Ptr conn);
    RPMItem(libdnf::utils::SQLite3Ptr conn, int64_t pk);
    virtual ~RPMItem() = default;

    const std::string &getName() const noexcept { return name; }
    void setName(const std::string &value) { name = value; }

    int32_t getEpoch() const noexcept { return epoch; }
    void setEpoch(int32_t value) { epoch = value; }

    const std::string &getVersion() const noexcept { return version; }
    void setVersion(const std::string &value) { version = value; }

    const std::string &getRelease() const noexcept { return release; }
    void setRelease(const std::string &value) { release = value; }

    const std::string &getArch() const noexcept { return arch; }
    void setArch(const std::string &value) { arch = value; }

    std::string getNEVRA() const;
    std::string toStr() const override;
    ItemType getItemType() const noexcept override { return itemType; }
    void save() override;

    static TransactionItemPtr getTransactionItem(libdnf::utils::SQLite3Ptr conn, const std::string &nevra);
    static std::vector< int64_t > searchTransactions(libdnf::utils::SQLite3Ptr conn, const std::vector< std::string > &patterns);
    static std::vector< TransactionItemPtr > getTransactionItems(libdnf::utils::SQLite3Ptr conn,
                                                                 int64_t transaction_id);
    static TransactionItemReason resolveTransactionItemReason(libdnf::utils::SQLite3Ptr conn,
                                                              const std::string &name,
                                                              const std::string &arch,
                                                              int64_t maxTransactionId);

    bool operator<(const RPMItem &other) const;

protected:
    const ItemType itemType = ItemType::RPM;
    std::string name;
    int32_t epoch = 0;
    std::string version;
    std::string release;
    std::string arch;

    void dbSelect(int64_t transaction_id);
    void dbInsert();
    void dbSelectOrInsert();
};

}  // namespace libdnf::transaction

#endif // LIBDNF_TRANSACTION_RPMITEM_HPP
