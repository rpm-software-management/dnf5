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

#include "module/module_item_container_impl.hpp"
#include "module/module_metadata.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/base/base.hpp"
#include "libdnf/base/base_weak.hpp"
#include "libdnf/module/module_errors.hpp"
#include "libdnf/module/module_item.hpp"
#include "libdnf/module/module_item_container_weak.hpp"

#include <modulemd-2.0/modulemd.h>

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
}

#include <memory>
#include <string>

namespace libdnf::module {


ModuleItemContainer::ModuleItemContainer(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}
ModuleItemContainer::~ModuleItemContainer() {}


const std::vector<std::unique_ptr<ModuleItem>> & ModuleItemContainer::get_modules() const {
    // TODO(mracek) What about to call add_modules_without_static_context before returning the vector?
    return p_impl->modules;
}

void ModuleItemContainer::add(const std::string & file_content, const std::string & repo_id) {
    ModuleMetadata md(get_base());
    try {
        md.add_metadata_from_string(file_content, 0);
    } catch (const ModuleResolveError & e) {
        throw ModuleResolveError(
            M_("Failed to load module metadata for repository \"{}\": {}"), repo_id, std::string(e.what()));
    }
    md.resolve_added_metadata();

    Repo * repo;
    auto repo_pair = p_impl->repositories.find(repo_id);
    if (repo_pair == p_impl->repositories.end()) {
        repo = repo_create(p_impl->pool, repo_id.c_str());
        p_impl->repositories[repo_id] = int(repo->repoid);
    } else {
        repo = pool_id2repo(p_impl->pool, Id(repo_pair->second));
    }
    auto items = md.get_all_module_items(get_weak_ptr(), repo_id);
    // Store module items with static context
    for (auto const & module_item_ptr : items.first) {
        std::unique_ptr<ModuleItem> module_item(module_item_ptr);
        p_impl->modules.push_back(std::move(module_item));
    }
    // Store module items without static context
    for (auto const & module_item_ptr : items.second) {
        std::unique_ptr<ModuleItem> module_item(module_item_ptr);
        modules_without_static_context.push_back(std::move(module_item));
    }
}


void ModuleItemContainer::add_modules_without_static_context() {
    if (modules_without_static_context.empty()) {
        return;
    }

    // Create a map based on modules with static context. For each "name:stream", map requires_strings to ModuleItems.
    std::map<std::string, std::map<std::string, std::vector<ModuleItem *>>> static_context_map;
    for (auto const & module_item : p_impl->modules) {
        auto requires_string = module_item->get_module_dependencies_string();
        static_context_map[module_item->get_name_stream()][requires_string].push_back(module_item.get());
    }

    // For each module with dynamic context, check whether its requires_string matches requires_string of any
    // static-context module with the same "name:stream" (i.e. if it's in the static_context_map). If so, assign it
    // the same static context.
    for (auto & module_item : modules_without_static_context) {
        auto requires_string = module_item->get_module_dependencies_string();

        auto stream_iterator = static_context_map.find(module_item->get_name_stream());
        if (stream_iterator != static_context_map.end()) {
            auto context_iterator = stream_iterator->second.find(requires_string);
            if (context_iterator != stream_iterator->second.end()) {
                module_item->computed_static_context = context_iterator->second[0]->get_context();
                p_impl->modules.push_back(std::move(module_item));
                continue;
            }
        }
        // If the requires_string didn't match anything in the static_context_map, set the new static context to
        // the requires_string (or "NoRequires" if empty). This means all dynamic-context modules with the same
        // "name:stream" and the same dependencies will have the same static context.
        if (requires_string.empty()) {
            requires_string.append("NoRequires");
        }
        module_item->computed_static_context = requires_string;
        p_impl->modules.push_back(std::move(module_item));
    }
    modules_without_static_context.clear();
    create_module_solvables();
}


void ModuleItemContainer::create_module_solvables() {
    for (auto const & module_item : p_impl->modules) {
        module_item->create_solvable();
        module_item->create_dependencies();
    }

    // TODO(pkratoch): Implement these calls (must be called lazy, before constructing goal or creating query for provides)
    // dnf_sack_set_provides_not_ready(moduleSack);
    // dnf_sack_set_considered_to_update(moduleSack);
}


ModuleItemContainerWeakPtr ModuleItemContainer::get_weak_ptr() {
    return ModuleItemContainerWeakPtr(this, &data_guard);
}


BaseWeakPtr ModuleItemContainer::get_base() const {
    return p_impl->base;
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
