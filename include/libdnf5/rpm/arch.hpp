// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_RPM_ARCH_HPP
#define LIBDNF5_RPM_ARCH_HPP

#include "libdnf5/defs.h"

#include <string>
#include <vector>


namespace libdnf5::rpm {

/// Returns a list of architectures supported by libdnf5.
LIBDNF_API std::vector<std::string> get_supported_arches();

/// Returns base architecture of the given `arch`. In case the base arch is not
/// found the function returns empty string.
/// @param arch Architecture.
LIBDNF_API std::string get_base_arch(const std::string & arch);

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_ARCH_HPP
