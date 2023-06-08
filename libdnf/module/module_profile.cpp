
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

#include "libdnf/module/module_profile.hpp"

#include "utils/string.hpp"

#include <modulemd-2.0/modulemd.h>

namespace libdnf5::module {


std::string ModuleProfile::get_name() const {
    if (!profile) {
        return {};
    }
    return libdnf::utils::string::c_to_str(modulemd_profile_get_name(profile));
}


std::string ModuleProfile::get_description() const {
    if (!profile) {
        return {};
    }
    return libdnf::utils::string::c_to_str(modulemd_profile_get_description(profile, NULL));
}


std::vector<std::string> ModuleProfile::get_rpms() const {
    if (!profile) {
        return {};
    }
    gchar ** c_rpms = modulemd_profile_get_rpms_as_strv(profile);

    std::vector<std::string> rpms;
    for (gchar ** item = c_rpms; *item; ++item) {
        rpms.emplace_back(*item);
        g_free(*item);
    }
    g_free(c_rpms);

    return rpms;
}


bool ModuleProfile::is_default() const {
    if (!profile) {
        return {};
    }
    return modulemd_profile_is_default(profile);
}


ModuleProfile::ModuleProfile(ModulemdProfile * profile) : profile(profile) {
    g_object_ref(profile);
}


ModuleProfile::ModuleProfile(const ModuleProfile & src) : profile(src.profile) {
    if (profile != nullptr) {
        g_object_ref(profile);
    }
}


ModuleProfile & ModuleProfile::operator=(const ModuleProfile & src) {
    if (this != &src) {
        g_object_unref(profile);
        profile = src.profile;
        if (profile != nullptr) {
            g_object_ref(profile);
        }
    }
    return *this;
}


ModuleProfile::~ModuleProfile() {
    if (profile != nullptr) {
        g_object_unref(profile);
    }
}


}  // namespace libdnf5::module
