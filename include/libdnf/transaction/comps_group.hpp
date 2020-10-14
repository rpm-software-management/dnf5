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


/// CompsGroup contains a copy of important data from comps::CompsGroup that is used
/// to perform comps transaction and then stored in the transaction (history) database.
///
/// @replaces libdnf:transaction/CompsGroupItem.hpp:class:CompsGroupItem
class CompsGroup : public TransactionItem {
public:
    explicit CompsGroup(Transaction & trans);

    /// Get text id of the group (xml element: <comps><group><id>VALUE</id>...)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getGroupId()
    const std::string & get_group_id() const noexcept { return group_id; }

    /// Get text id of the group (xml element: <comps><group><id>VALUE</id>...)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setGroupId(const std::string & value)
    void set_group_id(const std::string & value) { group_id = value; }

    /// Get name of the group (xml element: <comps><group><name>VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getName()
    const std::string & get_name() const noexcept { return name; }

    /// Set name of the group (xml element: <comps><group><name>VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setName(const std::string & value)
    void set_name(const std::string & value) { name = value; }

    /// Get translated name of the group in the current locale (xml element: <comps><group><name xml:lang="...">VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getTranslatedName()
    const std::string & get_translated_name() const noexcept { return translated_name; }

    /// Set translated name of the group in the current locale (xml element: <comps><group><name xml:lang="...">VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setTranslatedName(const std::string & value)
    void set_translated_name(const std::string & value) { translated_name = value; }

    /// Get types of the packages to be installed with the group (related xml elements: <comps><group><packagelist><packagereq type="VALUE" ...>)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getPackageTypes()
    CompsPackageType get_package_types() const noexcept { return package_types; }

    /// Set types of the packages to be installed with the group (related xml elements: <comps><group><packagelist><packagereq type="VALUE" ...>)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setPackageTypes(libdnf::CompsPackageType value)
    void set_package_types(CompsPackageType value) { package_types = value; }

    /// Create a new CompsGroupPackage object and return a reference to it.
    /// The object is owned by the CompsGroup.
    CompsGroupPackage & new_package();

    /// Get list of packages associated with the group.
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getPackages()
    const std::vector<std::unique_ptr<CompsGroupPackage>> & get_packages() { return packages; }

    // TODO(dmach): rewrite into TransactionSack.list_installed_groups(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf::utils::SQLite3Ptr conn,
    //    const std::string &pattern);

    /// Get string representation of the object, which equals to group_id
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.toStr()
    std::string to_string() const { return get_group_id(); }

private:
    friend class CompsGroupPackage;
    std::string group_id;
    std::string name;
    std::string translated_name;
    CompsPackageType package_types;
    std::vector<std::unique_ptr<CompsGroupPackage>> packages;
};


/// CompsGroupPackage represents a package associated with a comps group
///
/// @replaces libdnf:transaction/CompsGroupItem.hpp:class:CompsGroupPackage
class CompsGroupPackage {
public:
    /// Get database id (primary key)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getId()
    int64_t get_id() const noexcept { return id; }

    /// Set database id (primary key)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setId(int64_t value)
    void set_id(int64_t value) { id = value; }

    /// Get name of a package associated with a comps group (xml element: <comps><group><packagelist><packagereq>VALUE</packagereq>)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getName()
    const std::string & get_name() const noexcept { return name; }

    /// Set name of a package associated with a comps group (xml element: <comps><group><packagelist><packagereq>VALUE</packagereq>)
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setName(const std::string & value)
    void set_name(const std::string & value) { name = value; }

    /// Get a flag that determines if the package was present after the transaction it's associated with has finished.
    /// If the package was installed before running the transaction, it's still counted as installed.
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getInstalled()
    bool get_installed() const noexcept { return installed; }

    /// Set a flag that determines if the package was present after the transaction it's associated with has finished.
    /// If the package was installed before running the transaction, it's still counted as installed.
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setInstalled(bool value)
    void set_installed(bool value) { installed = value; }

    /// Get type of package associated with a comps group (xml element: <comps><group><packagelist><packagereq type="VALUE" ...>)
    /// See `enum class CompsPackageType` documentation for more details.
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getPackageType()
    CompsPackageType get_package_type() const noexcept { return package_type; }

    /// Set type of package associated with a comps group (xml element: <comps><group><packagelist><packagereq type="VALUE" ...>)
    /// See `enum class CompsPackageType` documentation for more details.
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setPackageType(libdnf::CompsPackageType value)
    void set_package_type(CompsPackageType value) { package_type = value; }

    /// Get the group the package is part of
    ///
    /// @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getGroup()
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
