// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
