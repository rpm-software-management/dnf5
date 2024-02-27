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

#ifndef LIBDNF5_MODULE_MODULE_SACK_IMPL_HPP
#define LIBDNF5_MODULE_MODULE_SACK_IMPL_HPP

#include "base/base_impl.hpp"
#include "module/module_db.hpp"
#include "module/module_metadata.hpp"
#include "solv/id_queue.hpp"
#include "solv/solv_map.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/module/module_sack.hpp"
#include "libdnf5/rpm/reldep_list.hpp"

extern "C" {
#include <solv/pool.h>
#include <solv/poolarch.h>
}

#include <optional>


namespace libdnf5::base {

class Transaction;

}

namespace libdnf5::module {


class ModuleGoalPrivate;


class ModuleSack::Impl {
public:
    /// Create libsolv pool and set the appropriate pool flags
    Impl(ModuleSack & module_sack, const BaseWeakPtr & base)
        : module_sack(&module_sack),
          base(base),
          module_metadata(base),
          pool(pool_create()),
          module_db(new ModuleDB(base)) {
        // Ensure excluded packages are not taken as candidates for solver
        pool_set_flag(pool, POOL_FLAG_WHATPROVIDESWITHDISABLED, 1);
        // Allow packages of the same name with different architectures to be installed in parallel
        pool_set_flag(pool, POOL_FLAG_IMPLICITOBSOLETEUSESCOLORS, 1);
    }
    ~Impl() {
        //TODO(amatej): Perhaps it would be good to create and use ModulePool which would take care of considered.
        //              It would also unify the approach with CompsPool and RpmPool.
        if (pool->considered) {
            map_free(pool->considered);
            g_free(pool->considered);
        }
        pool_free(pool);
    }

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

    void make_provides_ready();
    void recompute_considered_in_pool();

    // Requires resolved goal. Takes list_installs() from goal and adds all modules with the same SOLVABLE_NAME
    // (<name>:<stream>:<context>) into active_modules.
    void set_active_modules(ModuleGoalPrivate & goal);

    // Requires resolved goal. Enable all modules that are required by any newly enabled modules.
    // @replaces libdnf:ModulePackageContainer.hpp:method:ModulePackageContainer.enableDependencyTree()
    void enable_dependent_modules();

    /// Resolve given module items.
    ///
    /// @param module_items Module Items to resolve.
    /// @return `std::pair` of problems in resolving and ModuleErrorType.
    /// @since 5.0
    std::pair<std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>>, ModuleSack::ModuleErrorType> module_solve(
        std::vector<ModuleItem *> & module_items);

    /// Enable module stream.
    /// @param name module name to be enabled.
    /// @param stream module stream to be enabled.
    /// @param count if `true`, count the change towards the limit of module status modifications.
    /// @return `true` if requested change really triggers a change in the ModuleDB, `false` otherwise.
    /// @throw EnableMultipleStreamsError in case of conflicting enable requests.
    /// @throw NoModuleError if the module doesn't exist.
    /// @since 5.0.14
    bool enable(const std::string & name, const std::string & stream, bool count = true);
    /// Enable module stream.
    /// @param module_spec module to be enabled.
    /// @param count if `true`, count the change towards the limit of module status modifications.
    /// @return `true` if requested change realy triggers a change in the ModuleDB, `false` otherwise,
    ///         and a set of module:stream strings for modules with multiple streams and no enabled or default one.
    /// @throw NoModuleError if the module doesn't exist.
    /// @since 5.0.14
    std::pair<bool, std::set<std::string>> enable(const std::string & module_spec, bool count = true);
    /// Disable module.
    /// @param module_spec module to be disabled.
    /// @param count if `true`, count the change towards the limit of module status modifications.
    /// @throw NoModuleError if the module doesn't exist.
    /// @since 5.0.14
    bool disable(const std::string & module_spec, bool count = true);
    /// Reset module, so it's no longer enabled nor disabled.
    /// @param module_spec module to be reset.
    /// @param count if `true`, count the change towards the limit of module status modifications.
    /// @throw NoModuleError if the module doesn't exist.
    /// @since 5.0.14
    bool reset(const std::string & module_spec, bool count = true);
    /// Set architecture for module pool
    /// @param arch architecture.
    /// @since 5.1.8
    void set_arch(const char * arch) { pool_setarch(pool, arch); };


private:
    friend class libdnf5::base::Transaction;
    friend ModuleSack;
    friend ModuleItem;
    friend ModuleGoalPrivate;
    friend class ModuleQuery;

    ModuleSack * module_sack;
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
    bool considered_uptodate = false;
    bool platform_detected = false;

    std::map<std::string, std::string> module_defaults;
    std::unique_ptr<libdnf5::solv::SolvMap> excludes;
    std::map<Id, ModuleItem *> active_modules;
    std::vector<libdnf5::solv::IdQueue> modules_to_enable;

    std::unique_ptr<ModuleDB> module_db;

    /// @brief Method for autodetection of the platform id.
    /// @return If platform id was detected, it returns a pair where the first item is the platform
    ///         module name and second is the platform stream. Otherwise std::nullopt is returned.
    std::optional<std::pair<std::string, std::string>> detect_platform_name_and_stream() const;

    /// @brief Keep only one stream for each module. If more than one stream is originally there, keep only
    // the enabled or default one if possible.
    /// @return Set of module:stream strings for modules with multiple streams and no enabled or default one.
    std::set<std::string> prune_module_dict(
        std::unordered_map<std::string, std::unordered_map<std::string, libdnf5::solv::IdQueue>> & module_dict);
};

inline const std::vector<std::unique_ptr<ModuleItem>> & ModuleSack::Impl::get_modules() {
    add_modules_without_static_context();
    return modules;
}

}  // namespace libdnf5::module

#endif  // LIBDNF5_MODULE_MODULE_SACK_IMPL_HPP
