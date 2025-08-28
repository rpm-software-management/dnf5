// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_DEPRACATE_HPP
#define LIBDNF5_UTILS_DEPRACATE_HPP

#include <iostream>

#define LIBDNF5_DEPRECATED(msg)                                                     \
    {                                                                               \
        std::cerr << __PRETTY_FUNCTION__ << " is deprecated: " << msg << std::endl; \
    }

#endif  // LIBDNF5_UTILS_DEPRACATE_HPP
