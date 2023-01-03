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

#include "libdnf/module/module_item.hpp"

#include "module/module_sack_impl.hpp"
#include "utils/string.hpp"

#include "libdnf/module/module_dependency.hpp"
#include "libdnf/module/module_sack.hpp"
#include "libdnf/module/module_sack_weak.hpp"
#include "libdnf/utils/format.hpp"

#include <modulemd-2.0/modulemd-module-stream.h>
#include <modulemd-2.0/modulemd-profile.h>
#include <modulemd-2.0/modulemd.h>

extern "C" {
#include <solv/pool.h>
#include <solv/pool_parserpmrichdep.h>
#include <solv/repo.h>
}

#include <algorithm>
#include <string>
#include <vector>

namespace libdnf::module {


const char * ModuleItem::get_name_cstr() const {
    return modulemd_module_stream_get_module_name(md_stream);
}


std::string ModuleItem::get_name() const {
    return libdnf::utils::string::c_to_str(modulemd_module_stream_get_module_name(md_stream));
}


const char * ModuleItem::get_stream_cstr() const {
    return modulemd_module_stream_get_stream_name(md_stream);
}


std::string ModuleItem::get_stream() const {
    return libdnf::utils::string::c_to_str(modulemd_module_stream_get_stream_name(md_stream));
}


long long ModuleItem::get_version() const {
    return (long long)modulemd_module_stream_get_version(md_stream);
}


std::string ModuleItem::get_version_str() const {
    return std::to_string(modulemd_module_stream_get_version(md_stream));
}


const char * ModuleItem::get_context_cstr() const {
    return modulemd_module_stream_get_context(md_stream);
}


std::string ModuleItem::get_context() const {
    return libdnf::utils::string::c_to_str(modulemd_module_stream_get_context(md_stream));
}


const char * ModuleItem::get_arch_cstr() const {
    return modulemd_module_stream_get_arch(md_stream);
}


std::string ModuleItem::get_arch() const {
    return libdnf::utils::string::c_to_str(modulemd_module_stream_get_arch(md_stream));
}


std::string ModuleItem::get_name_stream(ModulemdModuleStream * md_stream) {
    return libdnf::utils::sformat(
        "{}:{}",
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_module_name(md_stream)),
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_stream_name(md_stream)));
}


std::string ModuleItem::get_name_stream_version() const {
    return libdnf::utils::sformat(
        "{}:{}:{}",
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_module_name(md_stream)),
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_stream_name(md_stream)),
        std::to_string(modulemd_module_stream_get_version(md_stream)));
}


std::string ModuleItem::get_name_stream_staticcontext() const {
    // TODO(pkratoch): Find out what is the fastest way to concatenate strings.
    return libdnf::utils::sformat(
        "{}:{}:{}",
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_module_name(md_stream)),
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_stream_name(md_stream)),
        computed_static_context.empty() ? libdnf::utils::string::c_to_str(modulemd_module_stream_get_context(md_stream))
                                        : computed_static_context);
}


std::string ModuleItem::get_full_identifier() const {
    return libdnf::utils::sformat(
        "{}:{}:{}:{}:{}",
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_module_name(md_stream)),
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_stream_name(md_stream)),
        std::to_string(modulemd_module_stream_get_version(md_stream)),
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_context(md_stream)),
        libdnf::utils::string::c_to_str(modulemd_module_stream_get_arch(md_stream)));
}


std::string ModuleItem::get_summary() const {
    return libdnf::utils::string::c_to_str(
        modulemd_module_stream_v2_get_summary((ModulemdModuleStreamV2 *)md_stream, NULL));
}


std::string ModuleItem::get_description() const {
    return libdnf::utils::string::c_to_str(
        modulemd_module_stream_v2_get_description((ModulemdModuleStreamV2 *)md_stream, NULL));
}


std::vector<std::string> ModuleItem::get_artifacts() const {
    std::vector<std::string> result_rpms;
    char ** rpms = modulemd_module_stream_v2_get_rpm_artifacts_as_strv((ModulemdModuleStreamV2 *)md_stream);

    for (char ** iter = rpms; iter && *iter; iter++) {
        result_rpms.emplace_back(std::string(*iter));
    }

    g_strfreev(rpms);
    return result_rpms;
}


std::vector<std::string> ModuleItem::get_demodularized_rpms() const {
    std::vector<std::string> result_rpms;
    char ** rpms = modulemd_module_stream_v2_get_demodularized_rpms((ModulemdModuleStreamV2 *)md_stream);

    for (char ** iter = rpms; iter && *iter; iter++) {
        result_rpms.emplace_back(std::string(*iter));
    }

    g_strfreev(rpms);
    return result_rpms;
}


std::vector<ModuleProfile> ModuleItem::get_profiles_internal(const char * name) const {
    std::vector<ModuleProfile> result_profiles;
    GPtrArray * profiles = modulemd_module_stream_v2_search_profiles((ModulemdModuleStreamV2 *)md_stream, name);

    for (unsigned int i = 0; i < profiles->len; i++) {
        result_profiles.push_back(ModuleProfile(static_cast<ModulemdProfile *>(g_ptr_array_index(profiles, i))));
    }

    g_ptr_array_free(profiles, true);
    return result_profiles;
}


bool ModuleItem::get_static_context() const {
    return modulemd_module_stream_v2_is_static_context((ModulemdModuleStreamV2 *)md_stream);
}


std::vector<ModuleDependency> ModuleItem::get_module_dependencies(
    ModulemdModuleStream * md_stream, bool remove_platform) {
    std::vector<ModuleDependency> dependencies;

    GPtrArray * c_dependencies = modulemd_module_stream_v2_get_dependencies((ModulemdModuleStreamV2 *)md_stream);

    for (unsigned int i = 0; i < c_dependencies->len; i++) {
        auto modulemd_dependencies = static_cast<ModulemdDependencies *>(g_ptr_array_index(c_dependencies, i));

        char ** runtime_req_modules = modulemd_dependencies_get_runtime_modules_as_strv(modulemd_dependencies);
        for (char ** iter_module = runtime_req_modules; iter_module && *iter_module; iter_module++) {
            std::string module_name = static_cast<char *>(*iter_module);
            if (remove_platform && module_name == "platform") {
                continue;
            }
            char ** runtime_req_streams =
                modulemd_dependencies_get_runtime_streams_as_strv(modulemd_dependencies, *iter_module);
            std::vector<std::string> streams;
            for (char ** iter_stream = runtime_req_streams; iter_stream && *iter_stream; iter_stream++) {
                streams.emplace_back(*iter_stream);
            }
            g_strfreev(runtime_req_streams);
            dependencies.emplace_back(ModuleDependency(std::move(module_name), std::move(streams)));
        }
        g_strfreev(runtime_req_modules);
    }

    return dependencies;
}


std::string ModuleItem::get_module_dependencies_string(ModulemdModuleStream * md_stream, bool remove_platform) {
    std::vector<std::string> dependencies_result;

    for (auto dependency : get_module_dependencies(md_stream, remove_platform)) {
        dependencies_result.emplace_back(dependency.to_string());
    }

    std::sort(dependencies_result.begin(), dependencies_result.end());
    return utils::string::join(dependencies_result, ";");
}


ModuleItem::ModuleItem(
    _ModulemdModuleStream * md_stream, const ModuleSackWeakPtr & module_sack, const std::string & repo_id)
    : md_stream(md_stream),
      module_sack(module_sack),
      repo_id(repo_id) {
    if (md_stream != nullptr) {
        g_object_ref(md_stream);
    }
}


ModuleItem::ModuleItem(const ModuleItem & mpkg)
    : md_stream(mpkg.md_stream),
      module_sack(mpkg.module_sack),
      id(mpkg.id),
      repo_id(mpkg.repo_id),
      computed_static_context(mpkg.computed_static_context) {
    if (md_stream != nullptr) {
        g_object_ref(md_stream);
    }
}


ModuleItem & ModuleItem::operator=(const ModuleItem & mpkg) {
    if (this != &mpkg) {
        if (md_stream != nullptr) {
            g_object_unref(md_stream);
        }
        md_stream = mpkg.md_stream;
        if (md_stream != nullptr) {
            g_object_ref(md_stream);
        }
        module_sack = mpkg.module_sack;
        id = mpkg.id;
        repo_id = mpkg.repo_id;
        computed_static_context = mpkg.computed_static_context;
    }
    return *this;
}


ModuleItem::ModuleItem(ModuleItem && mpkg)
    : md_stream(mpkg.md_stream),
      module_sack(std::move(mpkg.module_sack)),
      id(std::move(mpkg.id)),
      repo_id(std::move(mpkg.repo_id)),
      computed_static_context(std::move(mpkg.computed_static_context)) {
    mpkg.md_stream = nullptr;
}


ModuleItem & ModuleItem::operator=(ModuleItem && mpkg) {
    if (this != &mpkg) {
        if (md_stream != nullptr) {
            g_object_unref(md_stream);
        }
        md_stream = mpkg.md_stream;
        mpkg.md_stream = nullptr;
        module_sack = std::move(mpkg.module_sack);
        id = std::move(mpkg.id);
        repo_id = std::move(mpkg.repo_id);
        computed_static_context = std::move(mpkg.computed_static_context);
    }
    return *this;
}


ModuleItem::~ModuleItem() {
    if (md_stream != nullptr) {
        g_object_unref(md_stream);
    }
}


std::string ModuleItem::get_yaml() const {
    ModulemdModuleIndex * i = modulemd_module_index_new();
    modulemd_module_index_add_module_stream(i, md_stream, NULL);
    gchar * cStrYaml = modulemd_module_index_dump_to_string(i, NULL);
    std::string yaml = std::string(cStrYaml);
    g_free(cStrYaml);
    g_object_unref(i);
    return yaml;
}


void ModuleItem::create_solvable() {
    Pool * pool = module_sack->p_impl->pool;

    // Create new solvable and store its id
    id = ModuleItemId(repo_add_solvable(pool_id2repo(pool, Id(module_sack->p_impl->repositories[repo_id]))));
    Solvable * solvable = pool_id2solvable(pool, id.id);

    // Name: $name:$stream:$context
    solvable_set_str(solvable, SOLVABLE_NAME, get_name_stream_staticcontext().c_str());
    // Version: $version
    solvable_set_str(solvable, SOLVABLE_EVR, get_version_str().c_str());
    // TODO(pkratoch): The test can be removed once modules always have arch
    // Arch: $arch (if arch is not defined, set "noarch")
    auto arch = modulemd_module_stream_get_arch(md_stream);
    solvable_set_str(solvable, SOLVABLE_ARCH, arch ? arch : "noarch");
    // Store original $name:$stream in description
    solvable_set_str(solvable, SOLVABLE_DESCRIPTION, get_name_stream().c_str());

    // Create Provides: module($name)
    std::string provide = libdnf::utils::sformat("module({})", get_name());
    auto dep_id = pool_str2id(pool, provide.c_str(), 1);
    solvable_add_deparray(solvable, SOLVABLE_PROVIDES, dep_id, -1);
    // Create Conflicts: module($name)
    solvable_add_deparray(solvable, SOLVABLE_CONFLICTS, dep_id, 0);
    // Create Provides: module($name:$stream)
    provide = libdnf::utils::sformat("module({})", get_name_stream());
    dep_id = pool_str2id(pool, provide.c_str(), 1);
    solvable_add_deparray(solvable, SOLVABLE_PROVIDES, dep_id, -1);

    // TODO(pkratoch): Maybe store original context in summary
    // solvable_set_str(solvable, SOLVABLE_SUMMARY, original_context.c_str());
}


void ModuleItem::create_dependencies() const {
    Pool * pool = module_sack->p_impl->pool;
    Solvable * solvable = pool_id2solvable(pool, id.id);
    std::string req_formatted;

    for (const auto & dependency : get_module_dependencies()) {
        auto module_name = dependency.get_module_name();
        std::vector<std::string> required_streams;
        for (const auto & stream : dependency.get_streams()) {
            // If the stream require starts with "-", create conflict with the stream, otherwise, remember the stream require
            if (stream.find('-', 0) != std::string::npos) {
                req_formatted = libdnf::utils::sformat("module({}:{}", module_name, stream.substr(1));
                solvable_add_deparray(solvable, SOLVABLE_CONFLICTS, pool_str2id(pool, req_formatted.c_str(), 1), 0);
            } else {
                req_formatted = libdnf::utils::sformat("module({}:{})", module_name, stream);
                required_streams.push_back(req_formatted);
            }
        }
        // If there are no required streams, require the whole module
        // If there is exactly one required stream, require the stream
        // If there are more required streams, add a rich dependency to require any of the streams
        if (required_streams.empty()) {
            req_formatted = libdnf::utils::sformat("module({})", module_name);
            solvable_add_deparray(solvable, SOLVABLE_REQUIRES, pool_str2id(pool, req_formatted.c_str(), 1), -1);
        } else if (required_streams.size() == 1) {
            solvable_add_deparray(solvable, SOLVABLE_REQUIRES, pool_str2id(pool, required_streams[0].c_str(), 1), -1);
        } else {
            req_formatted = libdnf::utils::sformat("({})", utils::string::join(required_streams, " or "));
            Id dep_id = pool_parserpmrichdep(pool, req_formatted.c_str());
            if (!dep_id) {
                throw std::runtime_error("Cannot parse module requires");
            }
            solvable_add_deparray(solvable, SOLVABLE_REQUIRES, dep_id, -1);
        }
    }
}


void ModuleItem::create_solvable_and_dependencies() {
    module_sack->p_impl->provides_ready = false;
    module_sack->p_impl->considered_uptodate = false;
    create_solvable();
    create_dependencies();
}


bool ModuleItem::is_active() const {
    if (!module_sack->active_modules_resolved) {
        module_sack->resolve_active_module_items();
    }
    return module_sack->p_impl->active_modules.contains(id.id);
}


}  // namespace libdnf::module
