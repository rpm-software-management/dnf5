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


#ifndef LIBDNF_RPM_PACKAGE_SACK_HPP
#define LIBDNF_RPM_PACKAGE_SACK_HPP

#include "package.hpp"
#include "package_set.hpp"
#include "reldep.hpp"

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <map>
#include <memory>
#include <string>


namespace libdnf {

class Goal;
class Swdb;

}  // namespace libdnf

namespace libdnf::advisory {

class Advisory;
class AdvisorySack;
class AdvisoryQuery;
class AdvisoryCollection;
class AdvisoryPackage;
class AdvisoryModule;
class AdvisoryReference;

}  // namespace libdnf::advisory

namespace libdnf::rpm::solv {

class SolvPrivate;

}  // namespace libdnf::rpm::solv

namespace libdnf::base {

class Transaction;

}  // namespace libdnf::base

namespace libdnf::rpm {

class PackageSet;
class PackageSack;
using PackageSackWeakPtr = WeakPtr<PackageSack, false>;

class PackageSack {
public:
    explicit PackageSack(const libdnf::BaseWeakPtr & base);
    explicit PackageSack(libdnf::Base & base);
    ~PackageSack();

    /// Create WeakPtr to PackageSack
    PackageSackWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;

    /// Returns number of solvables in pool.
    int get_nsolvables() const noexcept;

    /// Loads excluded and included package sets from the configuration.
    /// Uses the `disable_excludes`, `excludepkgs`, and `includepkgs` configuration options for calculation.
    /// @param only_main If `true`, the repository specific configurations are not used.
    /// @since 5.0
    // TODO(mblaha): do we have a use case for only_main=true? Is the parameter needed?
    void load_config_excludes_includes(bool only_main = false);

    /// Returns user excluded package set
    const PackageSet get_user_excludes();

    /// Add package set to user excluded packages
    /// @param excludes: packages to add to excludes
    /// @since 5.0
    void add_user_excludes(const PackageSet & excludes);

    /// Remove package set from user excluded packages
    /// @param excludes: packages to remove from excludes
    /// @since 5.0
    void remove_user_excludes(const PackageSet & excludes);

    /// Resets user excluded packages to a new value
    /// @param excludes: packages to exclude
    /// @since 5.0
    void set_user_excludes(const PackageSet & excludes);

    /// Clear user excluded packages
    /// @since 5.0
    void clear_user_excludes();

    /// Returns user included package set
    const PackageSet get_user_includes();

    /// Add package set to user included packages
    /// @param includes: packages to add to includes
    /// @since 5.0
    void add_user_includes(const PackageSet & includes);

    /// Remove package set from user included packages
    /// @param includes: packages to remove from includes
    /// @since 5.0
    void remove_user_includes(const PackageSet & includes);

    /// Resets user included packages to a new value
    /// @param includes: packages to include
    /// @since 5.0
    void set_user_includes(const PackageSet & includes);

    /// Clear user included packages
    /// @since 5.0
    void clear_user_includes();

    rpm::Package get_running_kernel();

private:
    friend libdnf::Goal;
    friend Package;
    friend PackageSet;
    friend Reldep;
    friend class ReldepList;
    friend class repo::Repo;
    friend class PackageQuery;
    friend class Transaction;
    friend libdnf::Swdb;
    friend solv::SolvPrivate;
    friend libdnf::advisory::Advisory;
    friend libdnf::advisory::AdvisorySack;
    friend libdnf::advisory::AdvisoryQuery;
    friend libdnf::advisory::AdvisoryCollection;
    friend libdnf::advisory::AdvisoryPackage;
    friend libdnf::advisory::AdvisoryModule;
    friend libdnf::advisory::AdvisoryReference;
    friend libdnf::base::Transaction;

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_PACKAGE_SACK_HPP
