// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_BASE_PRIVATE_HPP
#define LIBDNF5_BASE_BASE_PRIVATE_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/exception.hpp"

#define libdnf_assert_same_base(base_a, base_b) \
    libdnf_assert(                              \
        (base_a) == (base_b), "Performing an operation on two objects instantiated from different Base instances");

#endif
