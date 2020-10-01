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


#ifndef LIBDNF_TRANSACTION_COMPS_ENVIRONMENT_HPP
#define LIBDNF_TRANSACTION_COMPS_ENVIRONMENT_HPP


#include <memory>
#include <vector>


namespace libdnf::transaction {

class CompsEnvironment;
typedef std::shared_ptr< CompsEnvironment > CompsEnvironmentPtr;

}


#include "Item.hpp"
#include "comps_group.hpp"


namespace libdnf::transaction {


class CompsEnvironmentGroup;


class CompsEnvironment : public Item {
public:
    using Item::Item;
    explicit CompsEnvironment(Transaction & trans, int64_t pk);
    virtual ~CompsEnvironment() = default;

    const std::string & get_environment_id() const noexcept { return environment_id; }
    void set_environment_id(const std::string &value) { environment_id = value; }

    const std::string & get_name() const noexcept { return name; }
    void set_name(const std::string & value) { name = value; }

    const std::string & get_translated_name() const noexcept { return translated_name; }
    void set_translated_name(const std::string & value) { translated_name = value; }

    CompsPackageType get_package_types() const noexcept { return package_types; }
    void set_package_types(CompsPackageType value) { package_types = value; }

    std::string toStr() const override;
    Type getItemType() const noexcept override { return itemType; }
    void save() override;

    /// Create a new CompsEnvironmentGroup object and return a reference to it.
    /// The object is owned by the CompsEnvironment.
    CompsEnvironmentGroup & new_group();

    /// Get list of groups associated with the environment.
    const std::vector<std::unique_ptr<CompsEnvironmentGroup>> & get_groups() { return groups; }

    //static TransactionItemPtr getTransactionItem(libdnf::utils::SQLite3Ptr conn, const std::string &envid);
    // TODO(dmach): rewrite into TransactionSack.list_installed_environments(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf::utils::SQLite3Ptr conn,
    //    const std::string &pattern);
    static std::vector< TransactionItemPtr > getTransactionItems(Transaction & trans);

protected:
    const Type itemType = Type::ENVIRONMENT;
    void loadGroups();
    void dbSelect(int64_t pk);
    void dbInsert();

private:
    friend class CompsEnvironmentGroup;
    std::string environment_id;
    std::string name;
    std::string translated_name;
    CompsPackageType package_types = CompsPackageType::DEFAULT;
    std::vector<std::unique_ptr<CompsEnvironmentGroup>> groups;
};


class CompsEnvironmentGroup {
public:
    int64_t get_id() const noexcept { return id; }
    void set_id(int64_t value) { id = value; }

    const CompsEnvironment &get_environment() const noexcept { return environment; }

    const std::string & get_group_id() const noexcept { return group_id; }
    void set_group_id(const std::string & value) { group_id = value; }

    bool get_installed() const noexcept { return installed; }
    void set_installed(bool value) { installed = value; }

    CompsPackageType get_group_type() const noexcept { return group_type; }
    void set_group_type(CompsPackageType value) { group_type = value; }

protected:
    explicit CompsEnvironmentGroup(CompsEnvironment &environment);

private:
    friend class CompsEnvironment;
    int64_t id = 0;
    std::string group_id;
    bool installed = false;
    CompsPackageType group_type;
    CompsEnvironment & environment;
};


}  // namespace libdnf::transaction


#endif // LIBDNF_TRANSACTION_COMPS_ENVIRONMENT_HPP
