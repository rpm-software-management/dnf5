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

#ifndef LIBDNF_MODULE_MODULE_ITEM_CONTAINER_HPP
#define LIBDNF_MODULE_MODULE_ITEM_CONTAINER_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/module/module_item.hpp"

#include <memory>
#include <string>
#include <vector>

namespace libdnf::module {


// TODO(pkratoch): Make this a docstring.
// ENABLED - a module that has an enabled stream.
// DISABLED - a module that is disabled.
// DEFAULT - a module that has a default stream (but isn't ENABLED nor DISABLED).
// AVAILABLE - otherwise.
enum class ModuleState { AVAILABLE, DEFAULT, ENABLED, DISABLED };


class ModuleItemContainer;


using ModuleItemContainerWeakPtr = libdnf::WeakPtr<ModuleItemContainer, false>;


class ModuleItemContainer {
public:
    ModuleItemContainer(const BaseWeakPtr & base);
    ModuleItemContainer(Base & base);

    ModuleItemContainerWeakPtr get_weak_ptr();

    std::vector<std::unique_ptr<ModuleItem>> modules;

    // TODO(pkratoch): Maybe make this private later
    void add(const std::string & file_content);

    // TODO(pkratoch): Implement adding defaults from "/etc/dnf/modules.defaults.d/", which are defined by user.
    //                 They are added with priority 1000 after everything else is loaded.
    /// Add and resolve defaults.
    /// @since 5.0
    //
    // @replaces libdnf:ModuleItemContainer.hpp:method:ModuleItemContainer.addDefaultsFromDisk()
    // @replaces libdnf:ModuleItemContainer.hpp:method:ModuleItemContainer.moduleDefaultsResolve()
    void add_defaults_from_disk();

    // TODO(pkratoch): Implement getting default streams and profiles.
    /// @return Default stream for given module.
    /// @since 5.0
    const std::string & get_default_stream(const std::string & name) const;
    /// @return List of all default profiles for given module stream.
    /// @since 5.0
    std::vector<std::string> get_default_profiles(std::string module_name, std::string module_stream);

private:
    BaseWeakPtr base;
    WeakPtrGuard<ModuleItemContainer, false> data_guard;

    BaseWeakPtr get_base() const;
};


}  // namespace libdnf::module


#endif  // LIBDNF_MODULE_MODULE_ITEM_CONTAINER_HPP
