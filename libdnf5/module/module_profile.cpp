
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

#include "libdnf5/module/module_profile.hpp"

#include "utils/string.hpp"

#include <modulemd-2.0/modulemd.h>

namespace libdnf5::module {

class ModuleProfile::Impl {
public:
    Impl(_ModulemdProfile * profile, const bool is_default) : profile(profile), is_default_profile(is_default) {
        g_object_ref(profile);
    }

    ~Impl();
    Impl(const Impl & src);
    Impl & operator=(const Impl & src);
    Impl(Impl && src) noexcept;
    Impl & operator=(Impl && src) noexcept;

private:
    friend ModuleProfile;

    // @replaces libdnf:module:modulemd/ModuleProfile.hpp:attribute:ModuleProfile.profile
    _ModulemdProfile * profile{nullptr};
    bool is_default_profile = false;
};

ModuleProfile::Impl::~Impl() {
    if (profile != nullptr) {
        g_object_unref(profile);
    }
}

ModuleProfile::Impl::Impl(const Impl & src) : profile(src.profile), is_default_profile(src.is_default_profile) {
    if (profile != nullptr) {
        g_object_ref(profile);
    }
}

ModuleProfile::Impl & ModuleProfile::Impl::operator=(const Impl & src) {
    if (this != &src) {
        if (profile != nullptr) {
            g_object_unref(profile);
        }
        profile = src.profile;
        is_default_profile = src.is_default_profile;
        if (profile != nullptr) {
            g_object_ref(profile);
        }
    }
    return *this;
}

ModuleProfile::Impl::Impl(Impl && src) noexcept : profile(src.profile), is_default_profile(src.is_default_profile) {
    src.profile = nullptr;
}

ModuleProfile::Impl & ModuleProfile::Impl::operator=(Impl && src) noexcept {
    if (this != &src) {
        if (profile != nullptr) {
            g_object_unref(profile);
        }
        profile = src.profile;
        is_default_profile = src.is_default_profile;
        src.profile = nullptr;
    }
    return *this;
}

std::string ModuleProfile::get_name() const {
    if (!p_impl->profile) {
        return {};
    }
    return libdnf5::utils::string::c_to_str(modulemd_profile_get_name(p_impl->profile));
}


std::string ModuleProfile::get_description() const {
    if (!p_impl->profile) {
        return {};
    }
    return libdnf5::utils::string::c_to_str(modulemd_profile_get_description(p_impl->profile, NULL));
}


std::vector<std::string> ModuleProfile::get_rpms() const {
    if (!p_impl->profile) {
        return {};
    }
    gchar ** c_rpms = modulemd_profile_get_rpms_as_strv(p_impl->profile);

    std::vector<std::string> rpms;
    for (gchar ** item = c_rpms; *item; ++item) {
        rpms.emplace_back(*item);
        g_free(*item);
    }
    g_free(c_rpms);

    return rpms;
}


bool ModuleProfile::is_default() const {
    return p_impl->is_default_profile;
}


ModuleProfile::ModuleProfile(ModulemdProfile * profile, const bool is_default)
    : p_impl(std::make_unique<Impl>(profile, is_default)) {}


ModuleProfile::ModuleProfile(const ModuleProfile & src) : p_impl(new Impl(*src.p_impl)) {}
ModuleProfile::ModuleProfile(ModuleProfile && src) noexcept = default;


ModuleProfile & ModuleProfile::operator=(const ModuleProfile & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
ModuleProfile & ModuleProfile::operator=(ModuleProfile && src) noexcept = default;


ModuleProfile::~ModuleProfile() = default;


}  // namespace libdnf5::module
