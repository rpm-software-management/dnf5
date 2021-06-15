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

#include <libdnf/common/proc.hpp>
#include <unistd.h>

namespace microdnf {

bool am_i_root() noexcept {
    return geteuid() == 0;
}

uid_t get_login_uid() noexcept {
    static uid_t cached_uid = libdnf::INVALID_UID;
    if (cached_uid == libdnf::INVALID_UID) {
        cached_uid = libdnf::read_login_uid_from_proc();
        if (cached_uid == libdnf::INVALID_UID) {
            cached_uid = getuid();
        }
    }
    return cached_uid;
}

}  // namespace microdnf
