// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_UTILS_AUTH_HPP
#define LIBDNF5_UTILS_AUTH_HPP


namespace libdnf5::utils {

/// Returns "true" if program runs with effective user ID = 0
bool am_i_root() noexcept;

}  // namespace libdnf5::utils

#endif
