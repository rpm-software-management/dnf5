/*
Copyright (C) 2020 Red Hat, Inc.

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


#ifndef LIBDNF_RPM_SOLV_SACK_HPP
#define LIBDNF_RPM_SOLV_SACK_HPP

#include "libdnf/utils/exception.hpp"
#include "libdnf/utils/weak_ptr.hpp"

#include <memory>

namespace libdnf {

class Base;
class Goal;
class Swdb;

}  // namespace libdnf

namespace libdnf::rpm::solv {

class SolvPrivate;

}  // namespace libdnf::rpm::solv


namespace libdnf::rpm {


class PackageSet;


struct PackageId {
public:
    PackageId() = default;
    explicit PackageId(int id);

    bool operator==(const PackageId & other) const noexcept { return id == other.id; };
    bool operator!=(const PackageId & other) const noexcept { return id != other.id; };

    int id{0};
};

struct ReldepId {
public:
    ReldepId() = default;
    explicit ReldepId(int id);

    bool operator==(const ReldepId & other) const noexcept { return id == other.id; };
    bool operator!=(const ReldepId & other) const noexcept { return id != other.id; };

    int id{0};
};

inline PackageId::PackageId(int id) : id(id) {}

inline ReldepId::ReldepId(int id) : id(id) {}


// forward declarations
class Package;
class Reldep;
class ReldepList;
class Repo;
class SolvQuery;
class Transaction;

class SolvSack;

using SolvSackWeakPtr = WeakPtr<SolvSack, false>;

class SolvSack {
public:
    class Exception : public RuntimeError {
    public:
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::SolvSack"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "rpm::SolvSack exception"; }
    };

    class NoCapability : public Exception {
    public:
        using Exception::Exception;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::SolvSack"; }
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

    explicit SolvSack(Base & base);
    ~SolvSack();

    //TODO(jrohel): Provide/use configuration options for flags?
    /// Loads rpm::Repo into SolvSack.
    void load_repo(Repo & repo, LoadRepoFlags flags);

    /// Creates system repository and loads it into SolvSack. Only one system repository can be in SolvSack.
    void create_system_repo(bool build_cache = false);

    /// Adds the given .rpm file to the command line repo.
    /// When add_with_hdrid == true the rpm is loaded with additional flags (RPM_ADD_WITH_HDRID|RPM_ADD_WITH_SHA256SUM)
    /// It will calculate SHA256 checksum of header and store it in pool => Requires more CPU for loading
    /// When RPM is not accesible or corrupted it raises libdnf::RuntimeError
    /// Return added new Package
    /// @replaces libdnf/dnf_sack.h:function:dnf_sack_add_cmdline_package(DnfSack *sack, const char *fn)
    /// @replaces libdnf/dnf_sack.h:function:dnf_sack_add_cmdline_package_nochecksum(DnfSack *sack, const char *fn)
    /// @replaces hawkey:hawkey/Sack:method:add_cmdline_package()
    Package add_cmdline_package(const std::string & fn, bool add_with_hdrid);

    // TODO(jmracek) The method is highly experimental, what about to move it somewhere else?
    /// Adds the given .rpm file to the system repo. The function is mostly for testing purpose (not only for unittests)
    /// When add_with_hdrid == true the rpm is loaded with additional flags (RPM_ADD_WITH_HDRID|RPM_ADD_WITH_SHA256SUM)
    /// It will calculate SHA256 checksum of header and store it in pool => Requires more CPU for loading
    /// When RPM is not accesible or corrupted it raises libdnf::RuntimeError
    /// Return added new Package
    Package add_system_package(const std::string & fn, bool add_with_hdrid, bool build_cache);

    // TODO (lhrazky): There's an overlap with dumping the debugdata on the Goal class
    void dump_debugdata(const std::string & dir);

    /// Create WeakPtr to SolvSack
    SolvSackWeakPtr get_weak_ptr();

private:
    friend libdnf::Goal;
    friend Package;
    friend PackageSet;
    friend Reldep;
    friend ReldepList;
    friend SolvQuery;
    friend Transaction;
    friend libdnf::Swdb;
    friend solv::SolvPrivate;
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

inline constexpr SolvSack::LoadRepoFlags operator|(SolvSack::LoadRepoFlags lhs, SolvSack::LoadRepoFlags rhs) {
    return static_cast<SolvSack::LoadRepoFlags>(
        static_cast<std::underlying_type_t<SolvSack::LoadRepoFlags>>(lhs) |
        static_cast<std::underlying_type_t<SolvSack::LoadRepoFlags>>(rhs));
}

inline constexpr SolvSack::LoadRepoFlags operator&(SolvSack::LoadRepoFlags lhs, SolvSack::LoadRepoFlags rhs) {
    return static_cast<SolvSack::LoadRepoFlags>(
        static_cast<std::underlying_type_t<SolvSack::LoadRepoFlags>>(lhs) &
        static_cast<std::underlying_type_t<SolvSack::LoadRepoFlags>>(rhs));
}

inline constexpr bool any(SolvSack::LoadRepoFlags flags) {
    return static_cast<typename std::underlying_type<SolvSack::LoadRepoFlags>::type>(flags) != 0;
}

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_SOLV_SACK_HPP
