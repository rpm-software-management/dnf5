// Copyright (C) 2025 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

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
