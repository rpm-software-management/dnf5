/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


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
