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

#ifndef LIBDNF_TRANSACTION_COMPSGROUPITEM_HPP
#define LIBDNF_TRANSACTION_COMPSGROUPITEM_HPP

#include <memory>
#include <vector>

namespace libdnf {

enum class CompsPackageType : int {
    CONDITIONAL = 1 << 0,
    DEFAULT = 1 << 1,
    MANDATORY = 1 << 2,
    OPTIONAL = 1 << 3
};

class CompsGroupItem;
class CompsGroupPackage;

typedef std::shared_ptr< CompsGroupItem > CompsGroupItemPtr;
typedef std::shared_ptr< CompsGroupPackage > CompsGroupPackagePtr;

}

#include "Item.hpp"
#include "TransactionItem.hpp"

namespace libdnf {

class CompsGroupItem : public Item {
public:
    explicit CompsGroupItem(SQLite3Ptr conn);
    CompsGroupItem(SQLite3Ptr conn, int64_t pk);
    virtual ~CompsGroupItem() = default;

    const std::string &getGroupId() const noexcept { return groupId; }
    void setGroupId(const std::string &value) { groupId = value; }

    const std::string &getName() const noexcept { return name; }
    void setName(const std::string &value) { name = value; }

    const std::string &getTranslatedName() const noexcept { return translatedName; }
    void setTranslatedName(const std::string &value) { translatedName = value; }

    CompsPackageType getPackageTypes() const noexcept { return packageTypes; }
    void setPackageTypes(CompsPackageType value) { packageTypes = value; }

    std::string toStr() const override;
    ItemType getItemType() const noexcept override { return itemType; }
    void save() override;
    CompsGroupPackagePtr addPackage(std::string name, bool installed, CompsPackageType pkgType);
    std::vector< CompsGroupPackagePtr > getPackages();
    static TransactionItemPtr getTransactionItem(SQLite3Ptr conn, const std::string &groupid);
    static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
        SQLite3Ptr conn,
        const std::string &pattern);
    static std::vector< TransactionItemPtr > getTransactionItems(SQLite3Ptr conn,
                                                                 int64_t transactionId);

protected:
    const ItemType itemType = ItemType::GROUP;
    std::string groupId;
    std::string name;
    std::string translatedName;
    CompsPackageType packageTypes;

    void loadPackages();
    std::vector< CompsGroupPackagePtr > packages;

private:
    friend class CompsGroupPackage;
    void dbSelect(int64_t pk);
    void dbInsert();
};

class CompsGroupPackage {
public:
    explicit CompsGroupPackage(CompsGroupItem &group);

    int64_t getId() const noexcept { return id; }
    void setId(int64_t value) { id = value; }

    const CompsGroupItem &getGroup() const noexcept { return group; }

    const std::string &getName() const noexcept { return name; }
    void setName(const std::string &value) { name = value; }

    bool getInstalled() const noexcept { return installed; }
    void setInstalled(bool value) { installed = value; }

    CompsPackageType getPackageType() const noexcept { return packageType; }
    void setPackageType(CompsPackageType value) { packageType = value; }

    // virtual std::string toStr();
    void save();

protected:
    int64_t id = 0;
    CompsGroupItem &group;
    std::string name;
    bool installed = false;
    CompsPackageType packageType = CompsPackageType::DEFAULT;

private:
    void dbInsert();
    void dbSelectOrInsert();
    void dbUpdate();
};

} // namespace libdnf

#endif // LIBDNF_TRANSACTION_COMPSGROUPITEM_HPP
