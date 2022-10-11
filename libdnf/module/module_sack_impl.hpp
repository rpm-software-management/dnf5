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

#ifndef LIBDNF_MODULE_MODULE_SACK_IMPL_HPP
#define LIBDNF_MODULE_MODULE_SACK_IMPL_HPP

#include "module/module_metadata.hpp"

#include "libdnf/module/module_sack.hpp"
#include "libdnf/rpm/reldep_list.hpp"

extern "C" {
#include <solv/pool.h>
}

namespace libdnf::module {


class ModuleSack::Impl {
public:
    Impl(const BaseWeakPtr & base) : base(base), module_metadata(base), pool(pool_create()) {}
    ~Impl() { pool_free(pool); }

    const std::vector<std::unique_ptr<ModuleItem>> & get_modules();

    /// Apply modular filtering to package set. For proper functionality, all repositories must be loaded and active
    /// modules must be resolved (modular solver).
    void module_filtering();

    /// Supporting method that iterates over all modules and creates filtering sets for modular filtering
    /// include_NEVRAs contains artifacts from active modules
    /// exclude_NEVRAs contains artifacts from not active modules
    /// names and reldep_name_list contain names from include_NEVRAs that are not source
    /// src_names contains names from include_NEVRAs that are source
    ///
    /// @return <include_NEVRAs>, <exclude_NEVRAs>, <names>, <src_names>, <reldep_name_list>
    std::tuple<
        std::vector<std::string>,
        std::vector<std::string>,
        std::vector<std::string>,
        std::vector<std::string>,
        rpm::ReldepList>
    collect_data_for_modular_filtering();

    // Compute static context for older modules and move these modules to `ModuleSack.modules`.
    void add_modules_without_static_context();

    void create_module_solvables();

    void make_provides_ready();

private:
    friend ModuleSack;
    friend ModuleItem;

    BaseWeakPtr base;
    ModuleMetadata module_metadata;
    std::vector<std::unique_ptr<ModuleItem>> modules;
    Pool * pool;
    // Repositories containing any modules. Key is repoid, value is Id of the repo in libsolv.
    // This is needed in `ModuleItem::create_solvable` for creating solvable in the correct repository.
    std::map<std::string, Id> repositories;

    // Older ModuleItems that don't have static context. After all metadata are loaded, static contexts are assigned
    // also to these ModuleItems and they are removed from this vector and added to `ModuleSack.modules`.
    // This is done in `ModuleSack::add_modules_without_static_context`.
    std::vector<std::unique_ptr<ModuleItem>> modules_without_static_context;

    bool provides_ready = false;
};

inline const std::vector<std::unique_ptr<ModuleItem>> & ModuleSack::Impl::get_modules() {
    add_modules_without_static_context();
    return modules;
}

}  // namespace libdnf::module

#endif  // LIBDNF_MODULE_MODULE_SACK_IMPL_HPP
