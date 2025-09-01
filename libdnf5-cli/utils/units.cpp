// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "libdnf5-cli/utils/units.hpp"

#include <fmt/format.h>


namespace libdnf5::cli::utils::units {


static const char * const SIZE_UNITS[] = {
    "B",
    "KiB",
    "MiB",
    "GiB",
    "TiB",
    "PiB",
    "EiB",
    "ZiB",
    "YiB",
};


std::pair<float, const char *> to_size(int64_t num) {
    auto value = static_cast<float>(num);
    int index = 0;
    while (value > 999 || value < -999) {
        value /= 1024;
        ++index;
    }
    return {value, SIZE_UNITS[index]};
}


std::string format_size_aligned(int64_t num) {
    auto [value, unit] = to_size(num);
    return fmt::format("{0:.1f} {1:>3s}", value, unit);
}


}  // namespace libdnf5::cli::utils::units
