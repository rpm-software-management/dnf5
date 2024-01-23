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

#ifndef LIBDNF5_TRANSACTION_COMPS_ENVIRONMENT_HPP
#define LIBDNF5_TRANSACTION_COMPS_ENVIRONMENT_HPP

#include "comps_group.hpp"

#include "libdnf5/comps/group/package.hpp"

#include <memory>
#include <vector>


namespace libdnf5::transaction {

class Transaction;
class CompsEnvironmentGroup;
class CompsEnvironmentDbUtils;
class CompsEnvironmentGroupDbUtils;


/// CompsEnvironment contains a copy of important data from comps::CompsEnvironment that is used
/// to perform comps transaction and then stored in the transaction (history) database.
///
// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:class:CompsEnvironmentItem
class CompsEnvironment : public TransactionItem {
public:
    /// Get string representation of the object, which equals to environment_id
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.toStr()
    std::string to_string() const { return get_environment_id(); }

    CompsEnvironment(const CompsEnvironment & src);
    CompsEnvironment & operator=(const CompsEnvironment & src);
    CompsEnvironment(CompsEnvironment && src) noexcept;
    CompsEnvironment & operator=(CompsEnvironment && src) noexcept;
    ~CompsEnvironment();

private:
    friend Transaction;
    friend CompsEnvironmentDbUtils;
    friend CompsEnvironmentGroupDbUtils;

    explicit CompsEnvironment(const Transaction & trans);

    /// Get text id of the environment (xml element: `<comps><environment><id>VALUE</id>...`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getEnvironmentId()
    const std::string & get_environment_id() const noexcept;

    /// Set text id of the environment (xml element: `<comps><environment><id>VALUE</id>...`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setEnvironmentId(const std::string & value)
    void set_environment_id(const std::string & value);

    /// Get name of the environment (xml element: `<comps><environment><name>VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getName()
    const std::string & get_name() const noexcept;

    /// Set name of the environment (xml element: `<comps><environment><name>VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setName(const std::string & value)
    void set_name(const std::string & value);

    /// Get translated name of the environment in the current locale (xml element: `<comps><environment><name xml:lang="...">VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getTranslatedName()
    const std::string & get_translated_name() const noexcept;

    /// Set translated name of the environment in the current locale (xml element: `<comps><environment><name xml:lang="...">VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setTranslatedName(const std::string & value)
    void set_translated_name(const std::string & value);

    /// Get types of the packages to be installed with the environment (related xml elements: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getPackageTypes()
    libdnf5::comps::PackageType get_package_types() const noexcept;

    /// Set types of the packages to be installed with the environment (related xml elements: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.setPackageTypes(libdnf::CompsPackageType value)
    void set_package_types(libdnf5::comps::PackageType value);

    /// Create a new CompsEnvironmentGroup object and return a reference to it.
    /// The object is owned by the CompsEnvironment.
    CompsEnvironmentGroup & new_group();

    /// Get list of groups associated with the environment.
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentItem.getGroups()
    std::vector<CompsEnvironmentGroup> & get_groups();

    // TODO(dmach): rewrite into TransactionSack.list_installed_environments(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf5::utils::SQLite3Ptr conn,
    //    const std::string &pattern);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};


// @replaces libdnf:transaction/CompsEnvironmentItem.hpp:class:CompsEnvironmentGroup
class CompsEnvironmentGroup {
public:
    ~CompsEnvironmentGroup();
    CompsEnvironmentGroup(const CompsEnvironmentGroup & src);
    CompsEnvironmentGroup & operator=(const CompsEnvironmentGroup & src);
    CompsEnvironmentGroup(CompsEnvironmentGroup && src) noexcept;
    CompsEnvironmentGroup & operator=(CompsEnvironmentGroup && src) noexcept;
    CompsEnvironmentGroup();

private:
    friend Transaction;
    friend CompsEnvironmentGroupDbUtils;

    /// Get database id (primary key)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getId()
    int64_t get_id() const noexcept;

    /// Set database id (primary key)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setId(int64_t value)
    void set_id(int64_t value);

    /// Get groupid of a group associated with a comps environment (xml element: `<comps><environment><grouplist><groupid>VALUE</groupid>`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getGroupId()
    const std::string & get_group_id() const noexcept;

    /// Set groupid of a group associated with a comps environment (xml element: `<comps><environment><grouplist><groupid>VALUE</groupid>`)
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setGroupId(const std::string & value)
    void set_group_id(const std::string & value);

    /// Get a flag that determines if the group was present after the transaction it's associated with has finished.
    /// If the group was installed before running the transaction, it's still counted as installed.
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getInstalled()
    bool get_installed() const noexcept;

    /// Set a flag that determines if the group was present after the transaction it's associated with has finished.
    /// If the group was installed before running the transaction, it's still counted as installed.
    ///
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setInstalled(bool value)
    void set_installed(bool value);

    // TODO(dmach): this is not entirely clear; investigate and document
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.getGroupType()
    libdnf5::comps::PackageType get_group_type() const noexcept;

    // TODO(dmach): this is not entirely clear; investigate and document
    // @replaces libdnf:transaction/CompsEnvironmentItem.hpp:method:CompsEnvironmentGroup.setGroupType(libdnf::CompsPackageType value)
    void set_group_type(libdnf5::comps::PackageType value);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_COMPS_ENVIRONMENT_HPP
