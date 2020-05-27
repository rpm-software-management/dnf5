/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "utils.hpp"

#include <pwd.h>
#include <unistd.h>

#include <cstdlib>

namespace microdnf {

bool am_i_root() noexcept {
    return geteuid() == 0;
}

namespace xdg {

std::filesystem::path get_user_home_dir() {
    if (char * dir = std::getenv("HOME")) {
        return std::filesystem::path(dir);
    }
    if (struct passwd * pw = getpwuid(getuid())) {
        return std::filesystem::path(pw->pw_dir);
    }
    throw std::runtime_error("get_home_dir(): Can't determine the user's home directory");
}

std::filesystem::path get_user_cache_dir() {
    if (char * dir = std::getenv("XDG_CACHE_HOME")) {
        return std::filesystem::path(dir);
    }
    return std::filesystem::path(get_user_home_dir()) / ".cache";
}

std::filesystem::path get_user_config_dir() {
    if (char * dir = std::getenv("XDG_CONFIG_HOME")) {
        return std::filesystem::path(dir);
    }
    return std::filesystem::path(get_user_home_dir()) / ".config";
}

std::filesystem::path get_user_data_dir() {
    if (char * dir = std::getenv("XDG_DATA_HOME")) {
        return std::filesystem::path(dir);
    }
    return std::filesystem::path(get_user_home_dir()) / ".local" / "share";
}

std::filesystem::path get_user_runtime_dir() {
    if (char * dir = std::getenv("XDG_RUNTIME_HOME")) {
        return std::filesystem::path(dir);
    }
    return get_user_cache_dir();
}

}  // namespace xdg

}  // namespace microdnf
