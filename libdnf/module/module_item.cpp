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

#include "utils/string.hpp"

#include "libdnf/utils/format.hpp"

#include <modulemd-2.0/modulemd-module-stream.h>
#include <modulemd-2.0/modulemd-profile.h>
#include <modulemd-2.0/modulemd.h>

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


std::vector<std::string> ModuleItem::get_requires(ModulemdModuleStream * md_stream, bool remove_platform) {
    std::vector<std::string> dependencies_result;

    GPtrArray * c_dependencies = modulemd_module_stream_v2_get_dependencies((ModulemdModuleStreamV2 *)md_stream);

    for (unsigned int i = 0; i < c_dependencies->len; i++) {
        auto dependencies = static_cast<ModulemdDependencies *>(g_ptr_array_index(c_dependencies, i));
        if (!dependencies) {
            continue;
        }
        char ** runtime_req_modules = modulemd_dependencies_get_runtime_modules_as_strv(dependencies);

        for (char ** iter_module = runtime_req_modules; iter_module && *iter_module; iter_module++) {
            char ** runtime_req_streams = modulemd_dependencies_get_runtime_streams_as_strv(dependencies, *iter_module);
            auto module_name = static_cast<char *>(*iter_module);
            if (remove_platform && strcmp(module_name, "platform") == 0) {
                g_strfreev(runtime_req_streams);
                continue;
            }
            std::vector<std::string> required_streams;
            for (char ** iter_stream = runtime_req_streams; iter_stream && *iter_stream; iter_stream++) {
                required_streams.push_back(*iter_stream);
            }
            if (required_streams.empty()) {
                dependencies_result.emplace_back(module_name);
            } else {
                std::sort(required_streams.begin(), required_streams.end());
                std::string required_streams_string = *required_streams.begin();
                for (auto iter = std::next(required_streams.begin()); iter != required_streams.end(); ++iter) {
                    required_streams_string.append(",");
                    required_streams_string.append(*iter);
                }
                dependencies_result.emplace_back(
                    libdnf::utils::sformat("{}:[{}]", module_name, required_streams_string));
            }
            g_strfreev(runtime_req_streams);
        }
        g_strfreev(runtime_req_modules);
    }

    return dependencies_result;
}


ModuleItem::ModuleItem(_ModulemdModuleStream * md_stream) : md_stream(md_stream) {
    if (md_stream != nullptr) {
        g_object_ref(md_stream);
    }
}


ModuleItem::ModuleItem(const ModuleItem & mpkg) : md_stream(mpkg.md_stream), id(mpkg.id), repo_id(mpkg.repo_id) {
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
        repo_id = mpkg.repo_id;
        id = mpkg.id;
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


}  // namespace libdnf::module
