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

#ifndef LIBDNF5_MODULE_MODULE_SACK_HPP
#define LIBDNF5_MODULE_MODULE_SACK_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/base/solver_problems.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/module/module_item.hpp"
#include "libdnf5/module/module_sack_weak.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace libdnf5 {

class Goal;

}

namespace libdnf5::base {

class Transaction;

}

namespace libdnf5::repo {

class Repo;
class RepoSack;

}  // namespace libdnf5::repo

namespace libdnf5::module {


// TODO(pkratoch): Make this a docstring.
// ENABLED - a module that has an enabled stream.
// DISABLED - a module that is disabled.
// AVAILABLE - otherwise.
enum class ModuleStatus { AVAILABLE, ENABLED, DISABLED };


/// Container with data and methods related to modules
class ModuleSack {
public:
    enum class ModuleErrorType {
        NO_ERROR = 0,
        INFO,
        /// Error in module defaults detected during resolvement of module dependencies
        ERROR_IN_DEFAULTS,
        /// Error detected during resolvement of module dependencies
        ERROR,
        /// Error detected during resolvement of module dependencies - Unexpected error!!!
        CANNOT_RESOLVE_MODULES,
        CANNOT_RESOLVE_MODULE_SPEC,
        CANNOT_ENABLE_MULTIPLE_STREAMS,
        CANNOT_MODIFY_MULTIPLE_TIMES_MODULE_STATUS,
        /// Problem with latest modules during resolvement of module dependencies
        ERROR_IN_LATEST
    };

    ~ModuleSack();

    ModuleSackWeakPtr get_weak_ptr();

    /// @return All module items.
    /// @since 5.0
    const std::vector<std::unique_ptr<ModuleItem>> & get_modules();
    /// @return Active module items. I.e. module items whose RPMs are included in the set of available packages.
    /// @since 5.0
    std::vector<ModuleItem *> get_active_modules();

    // TODO(pkratoch): Implement getting default streams and profiles.
    /// @return Default stream for given module.
    /// @since 5.0
    const std::string & get_default_stream(const std::string & name) const;
    /// @return List of all default profiles for given module stream.
    /// @since 5.0
    std::vector<std::string> get_default_profiles(std::string module_name, std::string module_stream);

    /// Resolve which module items are active. This means requesting all enabled streams or default streams (default
    /// streams only from not-enabled non-disabled modules), while excluding all disabled module streams.
    ///
    /// @return A pair of problems in resolving to report and ModuleErrorType.
    /// @since 5.0
    std::pair<base::SolverProblems, ModuleErrorType> resolve_active_module_items();

private:
    friend class libdnf5::Base;
    friend class libdnf5::Goal;
    friend class libdnf5::base::Transaction;
    friend class libdnf5::repo::Repo;
    friend class libdnf5::repo::RepoSack;
    friend ModuleItem;
    friend class ModuleGoalPrivate;
    friend class ModuleQuery;

    ModuleSack(const BaseWeakPtr & base);

    BaseWeakPtr get_base() const;

    /// Load information about modules from file to ModuleSack. It is critical to load all module information from
    /// all available repositories when modular metadata are available.
    void add(const std::string & file_content, const std::string & repo_id);

    // TODO(pkratoch): Implement adding defaults from "/etc/dnf/modules.defaults.d/", which are defined by user.
    //                 They are added with priority 1000 after everything else is loaded.
    /// Add and resolve defaults.
    /// @since 5.0
    //
    // @replaces libdnf:ModulePackageContainer.hpp:method:ModulePackageContainer.addDefaultsFromDisk()
    // @replaces libdnf:ModulePackageContainer.hpp:method:ModulePackageContainer.moduleDefaultsResolve()
    void add_defaults_from_disk();

    WeakPtrGuard<ModuleSack, false> data_guard;

    bool active_modules_resolved = false;

    class Impl;
    std::unique_ptr<Impl> p_impl;
};


class InvalidModuleStatus : public libdnf5::Error {
public:
    InvalidModuleStatus(const std::string & status);

    const char * get_domain_name() const noexcept override { return "libdnf5::module"; }
    const char * get_name() const noexcept override { return "InvalidModuleStatus"; }
};


std::string module_status_to_string(ModuleStatus status);
ModuleStatus module_status_from_string(const std::string & status);


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_SACK_HPP
