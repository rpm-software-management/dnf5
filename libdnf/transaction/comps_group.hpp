/*
Copyright (C) 2017-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_TRANSACTION_COMPS_GROUP_HPP
#define LIBDNF_TRANSACTION_COMPS_GROUP_HPP


#include <memory>
#include <vector>


namespace libdnf::transaction {

class CompsGroup;

typedef std::shared_ptr< CompsGroup > CompsGroupPtr;

enum class CompsPackageType : int {
    CONDITIONAL = 1 << 0,
    DEFAULT = 1 << 1,
    MANDATORY = 1 << 2,
    OPTIONAL = 1 << 3
};

}


#include "Item.hpp"
#include "transaction_item.hpp"
#include "libdnf/transaction/db/comps_group.hpp"


namespace libdnf::transaction {


class CompsGroupPackage;


class CompsGroup : public Item {
public:
    using Item::Item;
    CompsGroup(Transaction & trans, int64_t pk);
    virtual ~CompsGroup() = default;

    const std::string & get_group_id() const noexcept { return group_id; }
    void set_group_id(const std::string & value) { group_id = value; }

    const std::string & get_name() const noexcept { return name; }
    void set_name(const std::string & value) { name = value; }

    const std::string & get_translated_name() const noexcept { return translated_name; }
    void set_translated_name(const std::string & value) { translated_name = value; }

    CompsPackageType get_package_types() const noexcept { return package_types; }
    void set_package_types(CompsPackageType value) { package_types = value; }

    std::string toStr() const override { return group_id; }

    Type getItemType() const noexcept override { return itemType; }
    void save() override;

    /// Create a new CompsGroupPackage object and return a reference to it.
    /// The object is owned by the CompsGroup.
    CompsGroupPackage & new_package();

    /// Get list of packages associated with the group.
    const std::vector<std::unique_ptr<CompsGroupPackage>> & get_packages() { return packages; }

    //static TransactionItemPtr getTransactionItem(libdnf::utils::SQLite3Ptr conn, const std::string &groupid);
    // TODO(dmach): rewrite into TransactionSack.list_installed_groups(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf::utils::SQLite3Ptr conn,
    //    const std::string &pattern);
    static std::vector< TransactionItemPtr > getTransactionItems(Transaction & trans);

protected:
    const Type itemType = Type::GROUP;
    void dbSelect(int64_t pk);
    void dbInsert();

private:
    friend class CompsGroupPackage;
    std::string group_id;
    std::string name;
    std::string translated_name;
    CompsPackageType package_types;
    std::vector<std::unique_ptr<CompsGroupPackage>> packages;
};


class CompsGroupPackage {
public:
    int64_t get_id() const noexcept { return id; }
    void set_id(int64_t value) { id = value; }

    const std::string & get_name() const noexcept { return name; }
    void set_name(const std::string & value) { name = value; }

    bool get_installed() const noexcept { return installed; }
    void set_installed(bool value) { installed = value; }

    CompsPackageType get_package_type() const noexcept { return package_type; }
    void set_package_type(CompsPackageType value) { package_type = value; }

    const CompsGroup & get_group() const noexcept { return group; }

    void save();

protected:
    explicit CompsGroupPackage(CompsGroup & group);

private:
    friend class CompsGroup;
    int64_t id = 0;
    std::string name;
    bool installed = false;
    CompsPackageType package_type = CompsPackageType::DEFAULT;
    CompsGroup & group;
};


}  // namespace libdnf::transaction


#endif // LIBDNF_TRANSACTION_COMPS_GROUP_HPP
