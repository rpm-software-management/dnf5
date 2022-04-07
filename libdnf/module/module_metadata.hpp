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

#ifndef LIBDNF_MODULE_MODULE_METADATA_HPP
#define LIBDNF_MODULE_MODULE_METADATA_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/module/module_item.hpp"

#include <modulemd-2.0/modulemd-module-index.h>
#include <modulemd-2.0/modulemd.h>

#include <string>
#include <vector>

namespace libdnf::module {


class ModuleMetadata {
private:
    friend class ModuleItemContainer;

    ModuleMetadata(const BaseWeakPtr & base);
    ModuleMetadata(Base & base);
    ModuleMetadata(const ModuleMetadata & src);
    ModuleMetadata & operator=(const ModuleMetadata & src);
    ModuleMetadata(ModuleMetadata && src) = default;
    ModuleMetadata & operator=(ModuleMetadata && src) = default;
    ~ModuleMetadata();

    BaseWeakPtr base;

    ModulemdModuleIndex * resulting_module_index{nullptr};
    ModulemdModuleIndexMerger * module_merger{nullptr};

    BaseWeakPtr get_base() const;

    void add_metadata_from_string(const std::string & yaml, int priority);
    void resolve_added_metadata();

    std::pair<std::vector<ModuleItem *>, std::vector<ModuleItem *>> get_all_module_items();
};


}  // namespace libdnf::module


#endif  // LIBDNF_MODULE_MODULE_METADATA_HPP
