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


#include "transaction_item.hpp"

#include <memory>
#include <vector>


namespace libdnf::transaction {


class CompsGroupPackage;
class Transaction;


/// CompsPackageType determines when a package in a comps group gets installed.
///
/// Expected behavior:
/// * All comps operations are weak, if something is not available, it's silently skipped.
/// * If something is available but has unresolvable dependencies, an error is reported.
enum class CompsPackageType : int {
    CONDITIONAL = 1 << 0,  // a weak dependency
    DEFAULT = 1 << 1,      // installed by default, but can be unchecked in the UI
    MANDATORY = 1 << 2,    // installed
    OPTIONAL = 1 << 3      // not installed by default, but can be checked in the UI
};


class CompsGroup : public TransactionItem {
public:
    explicit CompsGroup(Transaction & trans);

    const std::string & get_group_id() const noexcept { return group_id; }
    void set_group_id(const std::string & value) { group_id = value; }

    const std::string & get_name() const noexcept { return name; }
    void set_name(const std::string & value) { name = value; }

    const std::string & get_translated_name() const noexcept { return translated_name; }
    void set_translated_name(const std::string & value) { translated_name = value; }

    CompsPackageType get_package_types() const noexcept { return package_types; }
    void set_package_types(CompsPackageType value) { package_types = value; }

    /// Create a new CompsGroupPackage object and return a reference to it.
    /// The object is owned by the CompsGroup.
    CompsGroupPackage & new_package();

    /// Get list of packages associated with the group.
    const std::vector<std::unique_ptr<CompsGroupPackage>> & get_packages() { return packages; }

    // TODO(dmach): rewrite into TransactionSack.list_installed_groups(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf::utils::SQLite3Ptr conn,
    //    const std::string &pattern);

    std::string to_string() const { return get_group_id(); }

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


#endif  // LIBDNF_TRANSACTION_COMPS_GROUP_HPP
