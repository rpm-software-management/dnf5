/*
Copyright (C) 2018-2020 Red Hat, Inc.

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

#ifndef LIBDNF_DEPENDENCY_HPP
#define LIBDNF_DEPENDENCY_HPP

#include "sack.hpp"

#include <memory>
#include <string>
#include <vector>


namespace libdnf::rpm {

/// @replaces libdnf/dnf-reldep.h:struct:DnfReldep
/// @replaces libdnf/repo/solvable/Dependency.hpp:struct:Dependency
/// @replaces hawkey:hawkey/reldep.py:class:Reldep
struct Dependency
{
public:
    /// @brief Creates a reldep from Id
    Dependency(Sack * sack, DependencyId DependencyId);

    /**
    * @brief Creates a reldep from name, version, and comparison type.
    *
    * @param sack p_sack: DnfSack*
    * @param name p_name: Required
    * @param version p_version: Can be also NULL
    * @param cmpType p_cmpType: Can be 0 or HY_EQ, HY_LT, HY_GT, and their combinations
    */
    Dependency(Sack * sack, const char * name, const char * version, int cmpType);

    /**
    * @brief Creates a reldep from Char*. If parsing fails it raises std::runtime_error.
    *
    * @param sack p_sack:...
    * @param dependency p_dependency:...
    */
    Dependency(Sack * sack, const std::string & dependency);
    Dependency(const Dependency &dependency);
    ~Dependency();

    const char *get_name() const;
    const char *get_relation() const;
    const char *get_version() const;
    const char *to_string() const;
    DependencyId get_id() const noexcept;

private:
    friend DependencyContainer;

    /**
    * @brief Returns Id of reldep
    *
    * @param sack p_sack: DnfSack*
    * @param name p_name: Required
    * @param version p_version: Can be also NULL
    * @param cmpType p_cmpType: Can be 0 or HY_EQ, HY_LT, HY_GT, and their combinations
    * @return Id
    */
    static DependencyId getReldepId(DnfSack *sack, const char *name, const char *version, int cmpType);

    /**
    * @brief Returns Id of reldep or raises std::runtime_error if parsing fails
    *
    * @param sack p_sack:DnfSack
    * @param reldepStr p_reldepStr: const Char* of reldep
    * @return Id
    */
    static DependencyId getReldepId(DnfSack *sack, const char * reldepStr);

    DnfSack * sack;
    DependencyId id;
};

inline DependencyId Dependency::getId() const noexcept { return id; }

}  // namespace libdnf::rpm

#endif //LIBDNF_DEPENDENCY_HPP
