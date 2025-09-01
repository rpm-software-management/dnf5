// Copyright Contributors to the DNF5 project.
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

#include "module/module_metadata.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/logger/logger.hpp"
#include "libdnf5/module/module_errors.hpp"
#include "libdnf5/module/module_sack_weak.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <modulemd-2.0/modulemd-module-index.h>
#include <modulemd-2.0/modulemd.h>

#include <string>
#include <tuple>
#include <utility>

namespace libdnf5::module {


ModuleMetadata::ModuleMetadata(const BaseWeakPtr & base) : base(base) {};
ModuleMetadata::ModuleMetadata(libdnf5::Base & base) : base(base.get_weak_ptr()) {}


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
        throw ModuleResolveError(M_("Failed to update from string: {}"), std::string(error->message));
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
    metadata_resolved = false;
}


void ModuleMetadata::resolve_added_metadata() {
    if (metadata_resolved || !module_merger) {
        return;
    }

    GError * error = NULL;

    resulting_module_index = modulemd_module_index_merger_resolve(module_merger, &error);
    if (error && !resulting_module_index) {
        throw ModuleResolveError(
            M_("Failed to resolve module metadata: {}"),
            (error->message) ? std::string(error->message) : "Unknown error");
    }
    if (error) {
        auto & logger = *base->get_logger();
        // TODO(pkratoch): Maybe use LogEvent instead
        logger.debug("There were errors while resolving modular defaults: {}", error->message);
    }

    modulemd_module_index_upgrade_defaults(resulting_module_index, MD_DEFAULTS_VERSION_ONE, &error);
    if (error) {
        throw ModuleResolveError(M_("Failed to upgrade defaults: {}"), std::string(error->message));
    }
    modulemd_module_index_upgrade_streams(resulting_module_index, MD_MODULESTREAM_VERSION_TWO, &error);
    if (error) {
        throw ModuleResolveError(M_("Failed to upgrade streams: {}"), std::string(error->message));
    }
    g_clear_pointer(&module_merger, g_object_unref);

    metadata_resolved = true;
}


std::pair<std::vector<ModuleItem *>, std::vector<ModuleItem *>> ModuleMetadata::get_all_module_items(
    const ModuleSackWeakPtr & module_sack, const std::string & repo_id) {
    resolve_added_metadata();

    std::vector<ModuleItem *> module_items;
    std::vector<ModuleItem *> module_items_without_static_context;
    if (!resulting_module_index) {  // no module metadata were added
        return std::make_pair(module_items, module_items_without_static_context);
    }

    GPtrArray * streams = modulemd_module_index_search_streams_by_nsvca_glob(resulting_module_index, NULL);
    for (unsigned int i = 0; i < streams->len; i++) {
        ModulemdModuleStream * modulemd_stream = static_cast<ModulemdModuleStream *>(g_ptr_array_index(streams, i));
        if (modulemd_module_stream_v2_is_static_context((ModulemdModuleStreamV2 *)modulemd_stream)) {
            module_items.push_back(new ModuleItem(modulemd_stream, module_sack, repo_id));
        } else {
            module_items_without_static_context.push_back(new ModuleItem(modulemd_stream, module_sack, repo_id));
        }
    }

    g_ptr_array_free(streams, TRUE);
    return std::make_pair(module_items, module_items_without_static_context);
}


std::map<std::string, std::string> ModuleMetadata::get_default_streams() {
    resolve_added_metadata();

    std::map<std::string, std::string> default_streams;
    if (!resulting_module_index) {  // no module metadata were added
        return default_streams;
    }

    GHashTable * table = modulemd_module_index_get_default_streams_as_hash_table(resulting_module_index, NULL);
    GHashTableIter iterator;
    gpointer key, value;
    g_hash_table_iter_init(&iterator, table);
    while (g_hash_table_iter_next(&iterator, &key, &value)) {
        default_streams[(char *)key] = (char *)value;
    }
    g_hash_table_unref(table);
    return default_streams;
}


std::vector<std::string> ModuleMetadata::get_default_profiles(std::string module_name, std::string module_stream) {
    resolve_added_metadata();

    std::vector<std::string> default_profiles;
    if (!resulting_module_index) {  // no module metadata were added
        return default_profiles;
    }

    ModulemdDefaultsV1 * defaults = (ModulemdDefaultsV1 *)modulemd_module_get_defaults(
        modulemd_module_index_get_module(resulting_module_index, module_name.c_str()));
    if (!defaults) {
        return default_profiles;
    }

    char ** list = modulemd_defaults_v1_get_default_profiles_for_stream_as_strv(defaults, module_stream.c_str(), NULL);

    for (char ** iter = list; iter && *iter; iter++) {
        default_profiles.emplace_back(*iter);
    }

    g_strfreev(list);
    return default_profiles;
}


}  // namespace libdnf5::module
