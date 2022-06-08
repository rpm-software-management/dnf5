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

#include "libdnf/module/module_item_container.hpp"

#include "module/module_metadata.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/base/base.hpp"
#include "libdnf/base/base_weak.hpp"
#include "libdnf/module/module_item.hpp"

#include <modulemd-2.0/modulemd.h>

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
}

#include <memory>
#include <string>

namespace libdnf::module {


ModuleItemContainer::ModuleItemContainer(const BaseWeakPtr & base) : base(base){};
ModuleItemContainer::ModuleItemContainer(libdnf::Base & base) : base(base.get_weak_ptr()) {}


void ModuleItemContainer::add(const std::string & file_content) {
    ModuleMetadata md(base);
    md.add_metadata_from_string(file_content, 0);
    md.resolve_added_metadata();

    // TODO(pkratoch): Implement compatibility for ModuleItems without static context
    auto items = md.get_all_module_items();
    for (auto const & module_item_ptr : items.first) {
        std::unique_ptr<ModuleItem> module_item(module_item_ptr);
        modules.push_back(std::move(module_item));
    }
    for (auto const & module_item_ptr : items.second) {
        std::unique_ptr<ModuleItem> module_item(module_item_ptr);
        modules.push_back(std::move(module_item));
    }
}


ModuleItemContainerWeakPtr ModuleItemContainer::get_weak_ptr() {
    return ModuleItemContainerWeakPtr(this, &data_guard);
}


BaseWeakPtr ModuleItemContainer::get_base() const {
    return base;
}


InvalidModuleState::InvalidModuleState(const std::string & state)
    : libdnf::Error(M_("Invalid module state: {}"), state) {}


std::string module_state_to_string(ModuleState state) {
    switch (state) {
        case ModuleState::AVAILABLE:
            return "Available";
        case ModuleState::DEFAULT:
            return "Default";
        case ModuleState::ENABLED:
            return "Enabled";
        case ModuleState::DISABLED:
            return "Disabled";
    }
    return "";
}


ModuleState module_state_from_string(const std::string & state) {
    if (state == "Available") {
        return ModuleState::AVAILABLE;
    } else if (state == "Default") {
        return ModuleState::DEFAULT;
    } else if (state == "Enabled") {
        return ModuleState::ENABLED;
    } else if (state == "Disabled") {
        return ModuleState::DISABLED;
    }

    throw InvalidModuleState(state);
}

}  // namespace libdnf::module
