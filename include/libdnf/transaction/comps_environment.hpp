/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
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

#include "comps_group.hpp"

#include <memory>
#include <vector>


namespace libdnf::transaction {

class CompsEnvironmentGroup;
class Transction;


/// CompsEnvironment contains a copy of important data from comps::CompsEnvironment that is used
/// to perform comps transaction and then stored in the transaction (history) database.
///
/// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:class:CompsEnvironmentItem
class CompsEnvironment : public TransactionItem {
public:
    explicit CompsEnvironment(Transaction & trans);

    /// Get text id of the environment (xml element: <comps><environment><id>VALUE</id>...)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getEnvironmentId()
    const std::string & get_environment_id() const noexcept { return environment_id; }

    /// Set text id of the environment (xml element: <comps><environment><id>VALUE</id>...)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setEnvironmentId(const std::string & value)
    void set_environment_id(const std::string & value) { environment_id = value; }

    /// Get name of the environment (xml element: <comps><environment><name>VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getName()
    const std::string & get_name() const noexcept { return name; }

    /// Set name of the environment (xml element: <comps><environment><name>VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setName(const std::string & value)
    void set_name(const std::string & value) { name = value; }

    /// Get translated name of the environment in the current locale (xml element: <comps><environment><name xml:lang="...">VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getTranslatedName()
    const std::string & get_translated_name() const noexcept { return translated_name; }

    /// Set translated name of the environment in the current locale (xml element: <comps><environment><name xml:lang="...">VALUE</name>...)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setTranslatedName(const std::string & value)
    void set_translated_name(const std::string & value) { translated_name = value; }

    /// Get types of the packages to be installed with the environment (related xml elements: <comps><group><packagelist><packagereq type="VALUE" ...>)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getPackageTypes()
    CompsPackageType get_package_types() const noexcept { return package_types; }

    /// Set types of the packages to be installed with the environment (related xml elements: <comps><group><packagelist><packagereq type="VALUE" ...>)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setPackageTypes(libdnf::CompsPackageType value)
    void set_package_types(CompsPackageType value) { package_types = value; }

    /// Create a new CompsEnvironmentGroup object and return a reference to it.
    /// The object is owned by the CompsEnvironment.
    CompsEnvironmentGroup & new_group();

    /// Get list of groups associated with the environment.
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getGroups()
    const std::vector<std::unique_ptr<CompsEnvironmentGroup>> & get_groups() { return groups; }

    // TODO(dmach): rewrite into TransactionSack.list_installed_environments(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf::utils::SQLite3Ptr conn,
    //    const std::string &pattern);


    /// Get string representation of the object, which equals to environment_id
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.toStr()
    std::string to_string() const { return get_environment_id(); }

private:
    friend class CompsEnvironmentGroup;
    std::string environment_id;
    std::string name;
    std::string translated_name;
    CompsPackageType package_types = CompsPackageType::DEFAULT;
    std::vector<std::unique_ptr<CompsEnvironmentGroup>> groups;
};


/// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:class:CompsEnvironmentGroup
class CompsEnvironmentGroup {
public:
    /// Get database id (primary key)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getId()
    int64_t get_id() const noexcept { return id; }

    /// Set database id (primary key)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setId(int64_t value)
    void set_id(int64_t value) { id = value; }

    /// Get groupid of a group associated with a comps environment (xml element: <comps><environment><grouplist><groupid>VALUE</groupid>)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getGroupId()
    const std::string & get_group_id() const noexcept { return group_id; }

    /// Set groupid of a group associated with a comps environment (xml element: <comps><environment><grouplist><groupid>VALUE</groupid>)
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setGroupId(const std::string & value)
    void set_group_id(const std::string & value) { group_id = value; }

    /// Get a flag that determines if the group was present after the transaction it's associated with has finished.
    /// If the group was installed before running the transaction, it's still counted as installed.
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getInstalled()
    bool get_installed() const noexcept { return installed; }

    /// Set a flag that determines if the group was present after the transaction it's associated with has finished.
    /// If the group was installed before running the transaction, it's still counted as installed.
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setInstalled(bool value)
    void set_installed(bool value) { installed = value; }

    // TODO(dmach): this is not entirely clear; investigate and document
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getGroupType()
    CompsPackageType get_group_type() const noexcept { return group_type; }

    // TODO(dmach): this is not entirely clear; investigate and document
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setGroupType(libdnf::CompsPackageType value)
    void set_group_type(CompsPackageType value) { group_type = value; }

    /// Get the environment the group is part of
    ///
    /// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getEnvironment()
    const CompsEnvironment & get_environment() const noexcept { return environment; }

protected:
    explicit CompsEnvironmentGroup(CompsEnvironment & environment);

private:
    friend class CompsEnvironment;
    int64_t id = 0;
    std::string group_id;
    bool installed = false;
    CompsPackageType group_type;
    CompsEnvironment & environment;
};

}  // namespace libdnf::transaction

#endif  // LIBDNF_TRANSACTION_COMPS_ENVIRONMENT_HPP
