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

#ifndef LIBDNF5_RPM_RELDEP_HPP
#define LIBDNF5_RPM_RELDEP_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/impl_ptr.hpp"

#include <memory>
#include <string>
#include <vector>


namespace libdnf5::rpm {

struct ReldepId {
public:
    ReldepId() = default;
    explicit ReldepId(int id) : id(id) {}

    bool operator==(const ReldepId & other) const noexcept { return id == other.id; };
    bool operator!=(const ReldepId & other) const noexcept { return id != other.id; };

    int id{0};
};

/// @brief Represent a relational dependency from libsolv
///
// @replaces libdnf/dnf-reldep.h:struct:DnfReldep
// @replaces libdnf/repo/solvable/Dependency.hpp:struct:Dependency
// @replaces hawkey:hawkey/__init__.py:class:Reldep
class Reldep {
public:
    enum class CmpType { NONE = 0, GT = (1 << 0), EQ = (1 << 1), GTE = (GT | EQ), LT = (1 << 2), LTE = (LT | EQ) };

    /// @brief Creates a reldep from Char*. If parsing fails it raises std::runtime_error.
    ///
    /// @param base Instance of Base class
    /// @param reldep_string String with the dependency
    // @replaces libdnf/repo/solvable/Dependency.hpp:method:Dependency(Sack * sack, const std::string & dependency)
    Reldep(const libdnf5::BaseWeakPtr & base, const std::string & reldep_string);
    Reldep(libdnf5::Base & base, const std::string & reldep_string);

    // @replaces libdnf/repo/solvable/Dependency.hpp:method:Dependency(const Dependency & dependency);
    Reldep(const Reldep & reldep);
    Reldep(Reldep && reldep) noexcept;

    // @replaces libdnf/repo/solvable/Dependency.hpp:method:~Dependency();
    // @replaces libdnf/dnf-reldep.h:function:dnf_reldep_free(DnfReldep *reldep)
    ~Reldep();

    bool operator==(const Reldep & other) const noexcept;
    bool operator!=(const Reldep & other) const noexcept;
    Reldep & operator=(const Reldep & other);
    Reldep & operator=(Reldep && other) = delete;

    // @replaces libdnf/repo/solvable/Dependency.hpp:method:getName()
    const char * get_name() const;

    // @replaces libdnf/repo/solvable/Dependency.hpp:method:getRelation()
    const char * get_relation() const;

    // @replaces libdnf/repo/solvable/Dependency.hpp:method:getVersion()
    const char * get_version() const;

    // @replaces libdnf/repo/solvable/Dependency.hpp:method:toString()
    // @replaces libdnf/dnf-reldep.h:function:dnf_reldep_to_string(DnfReldep *reldep)
    std::string to_string() const;

    /// Provide descriptive information about instance including string value and ID
    std::string to_string_description() const;

    // @replaces libdnf/repo/solvable/Dependency.hpp:method:getId()
    // @replaces libdnf/dnf-reldep.h:function:dnf_reldep_to_string(DnfReldep *reldep)
    ReldepId get_id() const noexcept;

    /// Return weak pointer to base
    BaseWeakPtr get_base() const;

    /// @brief Test if pattern is rich dependency
    /// Return true if pattern start with "("
    static bool is_rich_dependency(const std::string & pattern);

    /// Return unique ID representing Reldep
    int get_hash() const;

protected:
    /// @brief Creates a reldep from Id
    // @replaces libdnf/repo/solvable/Dependency.hpp:method:Dependency(Sack * sack, Id id)
    Reldep(const BaseWeakPtr & base, ReldepId dependency_id);

private:
    friend class ReldepList;
    friend class ReldepListIterator;

    /// @brief Creates a reldep from name, version, and comparison type.
    ///
    /// @param base: The Base
    /// @param name p_name: Required
    /// @param version p_version: Can be also NULL
    /// @param cmp_type p_cmpType: ComparisonType, and their combinations
    // @replaces libdnf/repo/solvable/Dependency.hpp:method:get_id()
    // @replaces libdnf/dnf-reldep.h:function:dnf_reldep_new(DnfSack *sack, const char *name, int cmp_type, const char *evr)
    Reldep(const BaseWeakPtr & base, const char * name, const char * version, CmpType cmp_type);

    /// @brief Returns Id of parsed reldep
    ///
    /// @param base: The Base
    /// @param name p_name: Required
    /// @param version p_version: Can be also NULL
    /// @param cmp_type p_cmpType: ComparisonType, and their combinations
    /// @param create Whether a new Id should be created when name does not exist
    /// @return DependencyId
    // @replaces libdnf/repo/solvable/Dependency.hpp:method:getReldepId(DnfSack *sack, const char *name, const char *version, int cmpType)
    static ReldepId get_reldep_id(
        const BaseWeakPtr & base, const char * name, const char * version, CmpType cmp_type, int create = 1);

    /// @brief Returns Id of reldep or raises std::runtime_error if parsing fails
    ///
    /// @param base: The Base
    /// @param reldep_str p_reldepStr: const Char* of reldep
    /// @return DependencyId
    /// @param create Whether a new Id should be created when name does not exist
    // @replaces libdnf/repo/solvable/Dependency.hpp:method:getReldepId(DnfSack *sack, const char * reldepStr)
    static ReldepId get_reldep_id(const BaseWeakPtr & base, const std::string & reldep_str, int create = 1);

    class Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_RELDEP_HPP
