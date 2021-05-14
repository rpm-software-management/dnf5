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


#ifndef LIBDNF_RPM_RELDEP_LIST_HPP
#define LIBDNF_RPM_RELDEP_LIST_HPP

#include "reldep.hpp"
#include "reldep_list_iterator.hpp"

#include <memory>

namespace libdnf::rpm {

// forward declarations
class Package;
class ReldepListIterator;
class SolvQuery;

/// @replaces libdnf/dnf-reldep-list.h:struct:DnfReldepList
/// @replaces libdnf/repo/solvable/DependencyContainer.hpp:struct:DependencyContainer
class ReldepList {
public:
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:DependencyContainer(const DependencyContainer &src)
    ReldepList(const ReldepList & src);

    ReldepList(ReldepList && src) noexcept;

    /// @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_new(DnfSack *sack)
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:DependencyContainer(DnfSack *sack)
    explicit ReldepList(const SolvSackWeakPtr & sack);

    /// @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_free(DnfReldepList *reldep_list)
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:~DependencyContainer()
    ~ReldepList();

    using iterator = ReldepListIterator;
    iterator begin() const;
    iterator end() const;

    bool operator==(const ReldepList & other) const noexcept;
    bool operator!=(const ReldepList & other) const noexcept;
    ReldepList & operator=(const ReldepList & src);
    ReldepList & operator=(ReldepList && src) noexcept;

    /// @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_add(DnfReldepList *reldep_list, DnfReldep *reldep)
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:add(Dependency *dependency)
    void add(Reldep & reldep);

    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:add(Id id))
    void add(ReldepId id);

    /// @brief Adds a reldep from Char*. Only globs in name are proccessed. The proccess is slow
    /// therefore if reldepStr is not a glob please use addReldep() instead.
    ///
    /// @param reldepStr p_reldepStr: Char*
    /// @return bool - false if parsing or reldep creation fails
    ///
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:addReldepWithGlob(const char *reldepStr)
    bool add_reldep_with_glob(const std::string & reldep_str);

    /// @brief Adds a reldep from Char*. It does not support globs.
    ///
    /// @param reldepStr p_reldepStr: Char*
    /// @return bool false if parsing or reldep creation fails
    ///
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:addReldep(const char *reldepStr)
    bool add_reldep(const std::string & reldep_str);

    /// @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_extend(DnfReldepList *rl1, DnfReldepList *rl2)
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:extend(DependencyContainer *container)
    void append(ReldepList & source);

    /// @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_index(DnfReldepList *reldep_list, gint index)
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:get(int index)
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:getPtr(int index)
    Reldep get(int index) const noexcept;

    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:getId(int index)
    ReldepId get_id(int index) const noexcept;

    /// @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_count(DnfReldepList *reldep_list)
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:count()
    int size() const noexcept;

    SolvSack * get_sack() const;

private:
    friend ReldepListIterator;
    friend Package;
    friend SolvQuery;

    /// @brief Adds a reldep from Char*. It does not support globs.
    ///
    /// @param reldep_str p_reldep_str: Char*
    /// @param create p_create: int 0 or 1 allowed. When 0 it will not create reldep for unknown name
    /// @return bool false if parsing or reldep creation fails
    ///
    /// @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:addReldep(const char *reldepStr)
    bool add_reldep(const std::string & reldep_str, int create);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};


inline bool ReldepList::add_reldep(const std::string & reldep_str) {
    return add_reldep(reldep_str, 1);
}


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_RELDEP_LIST_HPP
