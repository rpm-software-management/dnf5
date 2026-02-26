// Copyright Contributors to the DNF5 project.
// Copyright (C) 2025 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/utils/bootc.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <sys/statvfs.h>
#include <unistd.h>

#include <cerrno>
#include <filesystem>

namespace libdnf5::utils::bootc {

bool is_bootc_system() {
    const std::filesystem::path ostree_booted{"/run/ostree-booted"};
    return std::filesystem::is_regular_file(ostree_booted);
}

bool is_writable() {
    struct statvfs vfs;
    if (statvfs("/usr", &vfs) != 0) {
        throw libdnf5::SystemError(errno, M_("Failed to stat /usr filesystem"));
    }
    return (vfs.f_flag & ST_RDONLY) == 0;
}

}  // namespace libdnf5::utils::bootc
