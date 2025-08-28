// Copyright (C) 2025 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef LIBDNF5_BOOTC_HPP
#define LIBDNF5_BOOTC_HPP

#include "libdnf5/defs.h"

namespace libdnf5::utils::bootc {

LIBDNF_API bool is_bootc_system();

LIBDNF_API bool is_writable();

}  // namespace libdnf5::utils::bootc

#endif  // LIBDNF5_BOOTC_HPP
