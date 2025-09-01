// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_TRANSACTION_COMPS_GROUP_HPP
#define LIBDNF5_TRANSACTION_COMPS_GROUP_HPP

#include "transaction_item.hpp"

#include "libdnf5/comps/group/package.hpp"
#include "libdnf5/defs.h"

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
class LIBDNF_API CompsGroup : public TransactionItem {
public:
    /// Get string representation of the object, which equals to group_id
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.toStr()
    std::string to_string() const;

    /// Get types of the packages to be installed with the group (related xml elements: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getPackageTypes()
    libdnf5::comps::PackageType get_package_types() const noexcept;

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

    LIBDNF_LOCAL explicit CompsGroup(const Transaction & trans);

    /// Get text id of the group (xml element: `<comps><group><id>VALUE</id>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getGroupId()
    LIBDNF_LOCAL const std::string & get_group_id() const noexcept;

    /// Get text id of the group (xml element: `<comps><group><id>VALUE</id>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setGroupId(const std::string & value)
    LIBDNF_LOCAL void set_group_id(const std::string & value);

    /// Get name of the group (xml element: `<comps><group><name>VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getName()
    LIBDNF_LOCAL const std::string & get_name() const noexcept;

    /// Set name of the group (xml element: `<comps><group><name>VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setName(const std::string & value)
    LIBDNF_LOCAL void set_name(const std::string & value);

    /// Get translated name of the group in the current locale (xml element: `<comps><group><name xml:lang="...">VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getTranslatedName()
    LIBDNF_LOCAL const std::string & get_translated_name() const noexcept;

    /// Set translated name of the group in the current locale (xml element: `<comps><group><name xml:lang="...">VALUE</name>...`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setTranslatedName(const std::string & value)
    LIBDNF_LOCAL void set_translated_name(const std::string & value);

    /// Set types of the packages to be installed with the group (related xml elements: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.setPackageTypes(libdnf::CompsPackageType value)
    LIBDNF_LOCAL void set_package_types(libdnf5::comps::PackageType value);

    /// Create a new CompsGroupPackage object and return a reference to it.
    /// The object is owned by the CompsGroup.
    LIBDNF_LOCAL CompsGroupPackage & new_package();

    /// Get list of packages associated with the group.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupItem.getPackages()
    LIBDNF_LOCAL std::vector<CompsGroupPackage> & get_packages();

    // TODO(dmach): rewrite into TransactionSack.list_installed_groups(); how to deal with references to different transactions? We don't want all of them loaded into memory.
    //static std::vector< TransactionItemPtr > getTransactionItemsByPattern(
    //    libdnf5::utils::SQLite3Ptr conn,
    //    const std::string &pattern);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


/// CompsGroupPackage represents a package associated with a comps group
///
// @replaces libdnf:transaction/CompsGroupItem.hpp:class:CompsGroupPackage
class LIBDNF_API CompsGroupPackage {
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
    LIBDNF_LOCAL int64_t get_id() const noexcept;

    /// Set database id (primary key)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setId(int64_t value)
    LIBDNF_LOCAL void set_id(int64_t value);

    /// Get name of a package associated with a comps group (xml element: `<comps><group><packagelist><packagereq>VALUE</packagereq>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getName()
    LIBDNF_LOCAL const std::string & get_name() const noexcept;

    /// Set name of a package associated with a comps group (xml element: `<comps><group><packagelist><packagereq>VALUE</packagereq>`)
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setName(const std::string & value)
    LIBDNF_LOCAL void set_name(const std::string & value);

    /// Get a flag that determines if the package was present after the transaction it's associated with has finished.
    /// If the package was installed before running the transaction, it's still counted as installed.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getInstalled()
    LIBDNF_LOCAL bool get_installed() const noexcept;

    /// Set a flag that determines if the package was present after the transaction it's associated with has finished.
    /// If the package was installed before running the transaction, it's still counted as installed.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setInstalled(bool value)
    LIBDNF_LOCAL void set_installed(bool value);

    /// Get type of package associated with a comps group (xml element: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    /// See `enum class comps::PackageType` documentation for more details.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.getPackageType()
    LIBDNF_LOCAL libdnf5::comps::PackageType get_package_type() const noexcept;

    /// Set type of package associated with a comps group (xml element: `<comps><group><packagelist><packagereq type="VALUE" ...>`)
    /// See `enum class libdnf5::comps::PackageType` documentation for more details.
    ///
    // @replaces libdnf:transaction/CompsGroupItem.hpp:method:CompsGroupPackage.setPackageType(libdnf::PackageType value)
    LIBDNF_LOCAL void set_package_type(libdnf5::comps::PackageType value);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_COMPS_GROUP_HPP
