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

#include <unistd.h>

#include <filesystem>

namespace libdnf5::utils::bootc {

bool is_bootc_system() {
    const std::filesystem::path ostree_booted{"/run/ostree-booted"};
    return std::filesystem::is_regular_file(ostree_booted);
}

bool is_writable() {
    return access("/usr", W_OK) == 0;
}

}  // namespace libdnf5::utils::bootc
