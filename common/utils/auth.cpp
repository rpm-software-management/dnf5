// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "auth.hpp"

#include <unistd.h>

namespace libdnf5::utils {

bool am_i_root() noexcept {
    return geteuid() == 0;
}

}  // namespace libdnf5::utils
