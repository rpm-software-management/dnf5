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

namespace libdnf::repo {

class Repo;
class RepoSack;

}  // namespace libdnf::repo

namespace libdnf::rpm::solv {

class SolvPrivate;

}  // namespace libdnf::rpm::solv

namespace libdnf::base {

class Transaction;

}  // namespace libdnf::base

namespace libdnf::rpm {

class PackageSack;
using PackageSackWeakPtr = WeakPtr<PackageSack, false>;

class PackageSack {
public:
    class Exception : public RuntimeError {
    public:
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::PackageSack"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "rpm::PackageSack exception"; }
    };

    class NoCapability : public Exception {
    public:
        using Exception::Exception;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::PackageSack"; }
        const char * get_name() const noexcept override { return "NoCapability"; }
        const char * get_description() const noexcept override {
            return "repository does not provide required metadata type";
        }
    };

    // LoadRepoFlags:
    // @NONE:                    Load only main solvables
    // @USE_FILELISTS:           Add the filelists metadata
    // @USE_PRESTO:              Add presto deltas metadata
    // @USE_UPDATEINFO:          Add updateinfo metadata
    // @USE_OTHER:               Add other metadata
    enum class LoadRepoFlags {
        NONE = 0,
        USE_FILELISTS = 1 << 1,
        USE_PRESTO = 1 << 2,
        USE_UPDATEINFO = 1 << 3,
        USE_OTHER = 1 << 4,
    };

    explicit PackageSack(libdnf::Base & base);
    ~PackageSack();

    //TODO(jrohel): Provide/use configuration options for flags?
    /// Loads rpm::Repo into PackageSack.
    void load_repo(repo::Repo & repo, LoadRepoFlags flags);

    /// Creates system repository and loads it into PackageSack. Only one system repository can be in PackageSack.
    void create_system_repo(bool build_cache = false);

    /// Append a rpm database into system repository
    void append_extra_system_repo(const std::string & rootdir);

    /// Adds the given .rpm file to the command line repo.
    /// When add_with_hdrid == true the rpm is loaded with additional flags (RPM_ADD_WITH_HDRID|RPM_ADD_WITH_SHA256SUM)
    /// It will calculate SHA256 checksum of header and store it in pool => Requires more CPU for loading
    /// When RPM is not accesible or corrupted it raises libdnf::RuntimeError
    /// Return added new Package
    /// @replaces libdnf/dnf_sack.h:function:dnf_sack_add_cmdline_package(DnfSack *sack, const char *fn)
    /// @replaces libdnf/dnf_sack.h:function:dnf_sack_add_cmdline_package_nochecksum(DnfSack *sack, const char *fn)
    /// @replaces hawkey:hawkey/Sack:method:add_cmdline_package()
    libdnf::rpm::Package add_cmdline_package(const std::string & fn, bool add_with_hdrid);

    // TODO(jmracek) The method is highly experimental, what about to move it somewhere else?
    /// Adds the given .rpm file to the system repo. The function is mostly for testing purpose (not only for unittests)
    /// When add_with_hdrid == true the rpm is loaded with additional flags (RPM_ADD_WITH_HDRID|RPM_ADD_WITH_SHA256SUM)
    /// It will calculate SHA256 checksum of header and store it in pool => Requires more CPU for loading
    /// When RPM is not accesible or corrupted it raises libdnf::RuntimeError
    /// Return added new Package
    libdnf::rpm::Package add_system_package(const std::string & fn, bool add_with_hdrid, bool build_cache);

    // TODO (lhrazky): There's an overlap with dumping the debugdata on the Goal class
    void dump_debugdata(const std::string & dir);

    /// Create WeakPtr to PackageSack
    PackageSackWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() const;

    /// Returns number of solvables in pool.
    int get_nsolvables() const noexcept;

    /// @return Map of resolved reasons why packages were installed: ``{(name, arch) -> reason}``.
    ///         A package can be installed due to multiple reasons, only the most significant is returned.
    /// @since 5.0
    const std::map<std::pair<std::string, std::string>, libdnf::transaction::TransactionItemReason> & get_reasons()
        const {
        return reasons;
    }

private:
    friend libdnf::Goal;
    friend Package;
    friend PackageSet;
    friend Reldep;
    friend class ReldepList;
    friend class repo::RepoSack;
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
    std::map<std::pair<std::string, std::string>, libdnf::transaction::TransactionItemReason> reasons;
};

inline constexpr PackageSack::LoadRepoFlags operator|(PackageSack::LoadRepoFlags lhs, PackageSack::LoadRepoFlags rhs) {
    return static_cast<PackageSack::LoadRepoFlags>(
        static_cast<std::underlying_type_t<PackageSack::LoadRepoFlags>>(lhs) |
        static_cast<std::underlying_type_t<PackageSack::LoadRepoFlags>>(rhs));
}

inline constexpr PackageSack::LoadRepoFlags operator&(PackageSack::LoadRepoFlags lhs, PackageSack::LoadRepoFlags rhs) {
    return static_cast<PackageSack::LoadRepoFlags>(
        static_cast<std::underlying_type_t<PackageSack::LoadRepoFlags>>(lhs) &
        static_cast<std::underlying_type_t<PackageSack::LoadRepoFlags>>(rhs));
}

inline constexpr bool any(PackageSack::LoadRepoFlags flags) {
    return static_cast<typename std::underlying_type<PackageSack::LoadRepoFlags>::type>(flags) != 0;
}

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_PACKAGE_SACK_HPP
