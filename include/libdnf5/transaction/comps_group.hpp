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

#ifndef LIBDNF5_TRANSACTION_COMPS_GROUP_HPP
#define LIBDNF5_TRANSACTION_COMPS_GROUP_HPP

#include "transaction_item.hpp"

#include "libdnf5/comps/group/package.hpp"

#include <memory>
#include <vector>


namespace libdnf5::transaction {

class CompsGroupPackage;
class Transaction;
class CompsGroupDbUtils;
class CompsGroupPackageDbUtils;


/// CompsGroup contains a copy of important data from comps::CompsGroup that is used
/// to perform comps transaction and then stored in the transaction (history) database.
///
// @replaces libdnf:transaction/CompsGroupItem.hpp:class:CompsGroupItem
class CompsGroup : public TransactionItem {
public:
    /// Get string representation of the object, which equals to group_id
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.toStr()
    std::string to_string() const;

    CompsGroup(const CompsGroup & src);
    CompsGroup & operator=(const CompsGroup & src);
    CompsGroup(CompsGroup && src) noexcept;
    CompsGroup & operator=(CompsGroup && src) noexcept;
    ~CompsGroup();

private:
    friend Transaction;
    friend CompsGroupPackage;
    friend CompsGroupDbUtils;
    friend CompsGroupPackageDbUtils;

    explicit CompsGroup(const Transaction & trans);

    /// Get text id of the group (xml element: `<comps><group><id>VALUE</id>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getGroupId()
    const std::string & get_group_id() const noexcept;

    /// Get text id of the group (xml element: `<comps><group><id>VALUE</id>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setGroupId(const std::string & value)
    void set_group_id(const std::string & value);

    /// Get name of the group (xml element: `<comps><group><name>VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getName()
    const std::string & get_name() const noexcept;

    /// Set name of the group (xml element: `<comps><group><name>VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setName(const std::string & value)
    void set_name(const std::string & value);

    /// Get translated name of the group in the current locale (xml element: `<comps><group><name xml:lang="...">VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getTranslatedName()
    const std::string & get_translated_name() const noexcept;

    /// Set translated name of the group in the current locale (xml element: `<comps><group><name xml:lang="...">VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setTranslatedName(const std::string & value)
    void set_translated_name(const std::string & value);

    /// Get types of the packages to be installed with the group (related xml elements: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getPackageTypes()
    libdnf5::comps::PackageType get_package_types() const noexcept;

    /// Set types of the packages to be installed with the group (related xml elements: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setPackageTypes(libdnf::CompsPackageType value)
    void set_package_types(libdnf5::comps::PackageType value);

    /// Create a new CompsGroupPackage object and return a reference to it.
    /// The object is owned by the CompsGroup.
    CompsGroupPackage & new_package();

    /// Get list of packages associated with the group.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getPackages()
    std::vector<CompsGroupPackage> & get_packages();

    // TODO(dmach): rewrite into TransactionSack.list_installed_groups(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf5::utils::SQLite3Ptr conn,
    //    const std::string &pattern);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};


/// CompsGroupPackage represents a package associated with a comps group
///
// @replaces libdnf:transaction/CompsGroupItem.hpp:class:CompsGroupPackage
class CompsGroupPackage {
public:
    ~CompsGroupPackage();
    CompsGroupPackage(const CompsGroupPackage & src);
    CompsGroupPackage & operator=(const CompsGroupPackage & src);
    CompsGroupPackage(CompsGroupPackage && src) noexcept;
    CompsGroupPackage & operator=(CompsGroupPackage && src) noexcept;
    CompsGroupPackage();

private:
    friend Transaction;
    friend CompsGroupPackageDbUtils;

    /// Get database id (primary key)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getId()
    int64_t get_id() const noexcept;

    /// Set database id (primary key)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setId(int64_t value)
    void set_id(int64_t value);

    /// Get name of a package associated with a comps group (xml element: `<comps><group><packagelist><packagereq>VALUE</packagereq>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getName()
    const std::string & get_name() const noexcept;

    /// Set name of a package associated with a comps group (xml element: `<comps><group><packagelist><packagereq>VALUE</packagereq>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setName(const std::string & value)
    void set_name(const std::string & value);

    /// Get a flag that determines if the package was present after the transaction it's associated with has finished.
    /// If the package was installed before running the transaction, it's still counted as installed.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getInstalled()
    bool get_installed() const noexcept;

    /// Set a flag that determines if the package was present after the transaction it's associated with has finished.
    /// If the package was installed before running the transaction, it's still counted as installed.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setInstalled(bool value)
    void set_installed(bool value);

    /// Get type of package associated with a comps group (xml element: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    /// See `enum class comps::PackageType` documentation for more details.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getPackageType()
    libdnf5::comps::PackageType get_package_type() const noexcept;

    /// Set type of package associated with a comps group (xml element: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    /// See `enum class libdnf5::comps::PackageType` documentation for more details.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setPackageType(libdnf::PackageType value)
    void set_package_type(libdnf5::comps::PackageType value);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_COMPS_GROUP_HPP
