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

#include "module/module_metadata.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/base/base.hpp"
#include "libdnf/logger/logger.hpp"
#include "libdnf/module/module_errors.hpp"

#include <modulemd-2.0/modulemd-module-index.h>
#include <modulemd-2.0/modulemd.h>

#include <string>
#include <tuple>
#include <utility>

namespace libdnf::module {


ModuleMetadata::ModuleMetadata(const BaseWeakPtr & base) : base(base){};
ModuleMetadata::ModuleMetadata(libdnf::Base & base) : base(base.get_weak_ptr()) {}


ModuleMetadata::ModuleMetadata(const ModuleMetadata & src)
    : base(src.base),
      resulting_module_index(src.resulting_module_index),
      module_merger(src.module_merger) {
    if (resulting_module_index != nullptr) {
        g_object_ref(resulting_module_index);
    }
    if (module_merger != nullptr) {
        g_object_ref(module_merger);
    }
}


ModuleMetadata & ModuleMetadata::operator=(const ModuleMetadata & src) {
    if (this != &src) {
        base = src.base;
        if (resulting_module_index != nullptr) {
            g_object_unref(resulting_module_index);
        }
        if (module_merger != nullptr) {
            g_object_unref(module_merger);
        }
        resulting_module_index = src.resulting_module_index;
        module_merger = src.module_merger;
        if (resulting_module_index != nullptr) {
            g_object_ref(resulting_module_index);
        }
        if (module_merger != nullptr) {
            g_object_ref(module_merger);
        }
    }
    return *this;
}


ModuleMetadata::~ModuleMetadata() {
    if (resulting_module_index != nullptr) {
        g_object_unref(resulting_module_index);
    }
    if (module_merger != nullptr) {
        g_object_unref(module_merger);
    }
}


void ModuleMetadata::add_metadata_from_string(const std::string & yaml, int priority) {
    GError * error = NULL;
    g_autoptr(GPtrArray) failures = NULL;

    ModulemdModuleIndex * module_index = modulemd_module_index_new();
    gboolean success = modulemd_module_index_update_from_string(module_index, yaml.c_str(), FALSE, &failures, &error);
    if (!success) {
        auto & logger = *base->get_logger();
        for (unsigned int i = 0; i < failures->len; i++) {
            ModulemdSubdocumentInfo * item = (ModulemdSubdocumentInfo *)(g_ptr_array_index(failures, i));
            // TODO(pkratoch): Maybe use LogEvent instead
            logger.error("Module yaml error: {}", modulemd_subdocument_info_get_gerror(item)->message);
        }
    }
    if (error) {
        throw ModuleResolveError(M_("Failed to update from string: {}"), error->message);
    }
    if (!module_merger) {
        module_merger = modulemd_module_index_merger_new();
        if (resulting_module_index) {
            // Priority is set to 0 in order to use the current resulting_module_index data as a baseline
            modulemd_module_index_merger_associate_index(module_merger, resulting_module_index, 0);
            g_clear_pointer(&resulting_module_index, g_object_unref);
        }
    }

    modulemd_module_index_merger_associate_index(module_merger, module_index, priority);
    g_object_unref(module_index);
}


void ModuleMetadata::resolve_added_metadata() {
    if (!module_merger) {
        return;
    }

    GError * error = NULL;

    resulting_module_index = modulemd_module_index_merger_resolve(module_merger, &error);
    if (error && !resulting_module_index) {
        throw ModuleResolveError(M_("Failed to resolve: {}"), (error->message) ? error->message : "Unknown error");
    }
    if (error) {
        auto & logger = *base->get_logger();
        // TODO(pkratoch): Maybe use LogEvent instead
        logger.debug("There were errors while resolving modular defaults: {}", error->message);
    }

    modulemd_module_index_upgrade_defaults(resulting_module_index, MD_DEFAULTS_VERSION_ONE, &error);
    if (error) {
        throw ModuleResolveError(M_("Failed to upgrade defaults: {}"), error->message);
    }
    modulemd_module_index_upgrade_streams(resulting_module_index, MD_MODULESTREAM_VERSION_TWO, &error);
    if (error) {
        throw ModuleResolveError(M_("Failed to upgrade streams: {}"), error->message);
    }
    g_clear_pointer(&module_merger, g_object_unref);
}


std::pair<std::vector<ModuleItem *>, std::vector<ModuleItem *>> ModuleMetadata::get_all_module_items() {
    std::vector<ModuleItem *> module_items;
    std::vector<ModuleItem *> module_items_without_static_context;
    if (!resulting_module_index) {
        return std::make_pair(module_items, module_items_without_static_context);
    }

    GPtrArray * streams = modulemd_module_index_search_streams_by_nsvca_glob(resulting_module_index, NULL);
    for (unsigned int i = 0; i < streams->len; i++) {
        ModulemdModuleStream * modulemd_stream = static_cast<ModulemdModuleStream *>(g_ptr_array_index(streams, i));
        if (modulemd_module_stream_v2_is_static_context((ModulemdModuleStreamV2 *)modulemd_stream)) {
            module_items.push_back(new ModuleItem(modulemd_stream));
        } else {
            // TODO(pkratoch): Implement compatibility for ModuleItems without static context
            // TODO(pkratoch): In DNF4, the ModuleItem object was not immediatelly created for modules without static context,
            //                 but a tuple <repo, modulemd_stream, repo_id> was stored instead. Find out why.
            module_items_without_static_context.push_back(new ModuleItem(modulemd_stream));
        }
    }

    return std::make_pair(module_items, module_items_without_static_context);
}


}  // namespace libdnf::module
