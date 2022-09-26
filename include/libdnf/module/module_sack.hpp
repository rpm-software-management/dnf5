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

#ifndef LIBDNF_MODULE_MODULE_SACK_HPP
#define LIBDNF_MODULE_MODULE_SACK_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/module/module_item.hpp"
#include "libdnf/module/module_sack_weak.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace libdnf::repo {

class RepoSack;

}  // namespace libdnf::repo

namespace libdnf::module {


// TODO(pkratoch): Make this a docstring.
// ENABLED - a module that has an enabled stream.
// DISABLED - a module that is disabled.
// DEFAULT - a module that has a default stream (but isn't ENABLED nor DISABLED).
// AVAILABLE - otherwise.
enum class ModuleState { AVAILABLE, DEFAULT, ENABLED, DISABLED };


/// Container with data and methods related to modules
class ModuleSack {
public:
    ~ModuleSack();

    ModuleSackWeakPtr get_weak_ptr();

    /// Return module items in container
    const std::vector<std::unique_ptr<ModuleItem>> & get_modules();

    // TODO(pkratoch): Maybe make this private later
    void add(const std::string & file_content, const std::string & repo_id);
    // Compute static context for older modules and move these modules to `ModuleSack.modules`.
    void add_modules_without_static_context();

    // TODO(pkratoch): Implement adding defaults from "/etc/dnf/modules.defaults.d/", which are defined by user.
    //                 They are added with priority 1000 after everything else is loaded.
    /// Add and resolve defaults.
    /// @since 5.0
    //
    // @replaces libdnf:ModulePackageContainer.hpp:method:ModulePackageContainer.addDefaultsFromDisk()
    // @replaces libdnf:ModulePackageContainer.hpp:method:ModulePackageContainer.moduleDefaultsResolve()
    void add_defaults_from_disk();

    // TODO(pkratoch): Implement getting default streams and profiles.
    /// @return Default stream for given module.
    /// @since 5.0
    const std::string & get_default_stream(const std::string & name) const;
    /// @return List of all default profiles for given module stream.
    /// @since 5.0
    std::vector<std::string> get_default_profiles(std::string module_name, std::string module_stream);

private:
    friend class libdnf::Base;
    friend class libdnf::repo::RepoSack;
    friend ModuleItem;

    ModuleSack(const BaseWeakPtr & base);

    void create_module_solvables();
    BaseWeakPtr get_base() const;

    WeakPtrGuard<ModuleSack, false> data_guard;

    // Older ModuleItems that don't have static context. After all metadata are loaded, static contexts are assigned
    // also to these ModuleItems and they are removed from this vector and added to `ModuleSack.modules`.
    // This is done in `ModuleSack::add_modules_without_static_context`.
    std::vector<std::unique_ptr<ModuleItem>> modules_without_static_context;

    class Impl;
    std::unique_ptr<Impl> p_impl;
};


class InvalidModuleState : public libdnf::Error {
public:
    InvalidModuleState(const std::string & state);

    const char * get_domain_name() const noexcept override { return "libdnf::module"; }
    const char * get_name() const noexcept override { return "InvalidModuleState"; }
};


std::string module_state_to_string(ModuleState state);
ModuleState module_state_from_string(const std::string & state);


}  // namespace libdnf::module


#endif  // LIBDNF_MODULE_MODULE_SACK_HPP
