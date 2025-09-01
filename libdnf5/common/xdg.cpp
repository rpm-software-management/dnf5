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

#include "libdnf5/common/xdg.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <pwd.h>
#include <unistd.h>

#include <cstdlib>

namespace libdnf5::xdg {

std::filesystem::path get_user_home_dir() {
    const char * dir = std::getenv("HOME");
    if (dir && *dir != '\0') {
        auto ret = std::filesystem::path(dir);
        if (ret.is_absolute()) {
            return ret;
        }
    }
    if (struct passwd * pw = getpwuid(getuid())) {
        return std::filesystem::path(pw->pw_dir);
    }
    throw RuntimeError(M_("get_user_home_dir(): Cannot determine the user's home directory"));
}

std::filesystem::path get_user_cache_dir() {
    const char * dir = std::getenv("XDG_CACHE_HOME");
    if (dir && *dir != '\0') {
        auto ret = std::filesystem::path(dir);
        if (ret.is_absolute()) {
            return ret;
        }
    }
    return std::filesystem::path(get_user_home_dir()) / ".cache";
}

std::filesystem::path get_user_config_dir() {
    const char * dir = std::getenv("XDG_CONFIG_HOME");
    if (dir && *dir != '\0') {
        auto ret = std::filesystem::path(dir);
        if (ret.is_absolute()) {
            return ret;
        }
    }
    return std::filesystem::path(get_user_home_dir()) / ".config";
}

std::filesystem::path get_user_data_dir() {
    const char * dir = std::getenv("XDG_DATA_HOME");
    if (dir && *dir != '\0') {
        auto ret = std::filesystem::path(dir);
        if (ret.is_absolute()) {
            return ret;
        }
    }
    return std::filesystem::path(get_user_home_dir()) / ".local" / "share";
}

std::filesystem::path get_user_state_dir() {
    const char * dir = std::getenv("XDG_STATE_HOME");
    if (dir && *dir != '\0') {
        auto ret = std::filesystem::path(dir);
        if (ret.is_absolute()) {
            return ret;
        }
    }
    return std::filesystem::path(get_user_home_dir()) / ".local" / "state";
}

std::filesystem::path get_user_runtime_dir() {
    const char * dir = std::getenv("XDG_RUNTIME_DIR");
    if (dir && *dir != '\0') {
        auto ret = std::filesystem::path(dir);
        if (ret.is_absolute()) {
            return ret;
        }
    }
    throw RuntimeError(M_("get_user_runtime_dir(): Cannot determine the user's runtime directory"));
}

}  // namespace libdnf5::xdg
