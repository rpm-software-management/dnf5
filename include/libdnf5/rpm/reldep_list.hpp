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


#ifndef LIBDNF5_RPM_RELDEP_LIST_HPP
#define LIBDNF5_RPM_RELDEP_LIST_HPP

#include "reldep.hpp"
#include "reldep_list_iterator.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/defs.h"

#include <memory>


namespace libdnf5::rpm {

// @replaces libdnf/dnf-reldep-list.h:struct:DnfReldepList
// @replaces libdnf/repo/solvable/DependencyContainer.hpp:struct:DependencyContainer
class LIBDNF_API ReldepList {
public:
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:DependencyContainer(const DependencyContainer &src)
    ReldepList(const ReldepList & src);

    ReldepList(ReldepList && src) noexcept;

    // @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_new(DnfSack *sack)
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:DependencyContainer(DnfSack *sack)
    explicit ReldepList(const libdnf5::BaseWeakPtr & base);
    explicit ReldepList(libdnf5::Base & base);

    // @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_free(DnfReldepList *reldep_list)
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:~DependencyContainer()
    ~ReldepList();

    using iterator = ReldepListIterator;
    iterator begin() const;
    iterator end() const;

    bool operator==(const ReldepList & other) const noexcept;
    bool operator!=(const ReldepList & other) const noexcept;
    ReldepList & operator=(const ReldepList & src);
    ReldepList & operator=(ReldepList && src) noexcept;

    // @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_add(DnfReldepList *reldep_list, DnfReldep *reldep)
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:add(Dependency *dependency)
    void add(const Reldep & reldep);

    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:add(Id id))
    void add(ReldepId id);

    /// @brief Adds a reldep from Char*. Only globs in name are processed. The process is slow
    /// therefore if reldepStr is not a glob please use addReldep() instead.
    ///
    /// @param reldep_str p_reldepStr: Char*
    /// @return bool - false if parsing or reldep creation fails
    ///
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:addReldepWithGlob(const char *reldepStr)
    bool add_reldep_with_glob(const std::string & reldep_str);

    /// @brief Adds a reldep from Char*. It does not support globs.
    ///
    /// @param reldep_str p_reldepStr: Char*
    /// @return bool false if parsing or reldep creation fails
    ///
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:addReldep(const char *reldepStr)
    bool add_reldep(const std::string & reldep_str);

    // @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_extend(DnfReldepList *rl1, DnfReldepList *rl2)
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:extend(DependencyContainer *container)
    void append(ReldepList & source);

    // @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_index(DnfReldepList *reldep_list, gint index)
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:get(int index)
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:getPtr(int index)
    Reldep get(int index) const noexcept;

    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:getId(int index)
    ReldepId get_id(int index) const noexcept;

    // @replaces libdnf/dnf-reldep-list.h:function:dnf_reldep_list_count(DnfReldepList *reldep_list)
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:count()
    int size() const noexcept;

    /// Return true if container is empty
    bool empty() const noexcept;

    /// Remove all RelDeps
    void clear();

    libdnf5::BaseWeakPtr get_base() const;

private:
    friend ReldepListIterator;

    friend class Package;
    friend class PackageQuery;

    /// @brief Adds a reldep from Char*. It does not support globs.
    ///
    /// @param reldep_str p_reldep_str: Char*
    /// @param create p_create: int 0 or 1 allowed. When 0 it will not create reldep for unknown name
    /// @return bool false if parsing or reldep creation fails
    ///
    // @replaces libdnf/repo/solvable/DependencyContainer.hpp:method:addReldep(const char *reldepStr)
    LIBDNF_LOCAL bool add_reldep(const std::string & reldep_str, int create);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_RELDEP_LIST_HPP
