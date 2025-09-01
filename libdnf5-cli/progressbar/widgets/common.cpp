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


#include "common.hpp"

#include "libdnf5-cli/utils/units.hpp"

#include <fmt/format.h>


namespace libdnf5::cli::progressbar {


std::string format_size(int64_t num) {
    auto result = libdnf5::cli::utils::units::format_size_aligned(num);
    // add leading spaces up to 9 characters
    // to make sure that the formatted size has always the same length in the progressbar
    if (result.size() < 9) {
        result.insert(0, 9 - result.size(), ' ');
    }
    return result;
}


/// Display time in one of the following formats: MMmSSs / HHhMMm / DDdHHh
/// The explicit 'negative' argument allows displaying '-00m00s'.
std::string format_time(int64_t num, bool negative) {
    char negative_sign = negative ? '-' : ' ';

    // display [ -]MMmSSs
    int64_t seconds = std::abs(num) % 60;
    int64_t minutes = std::abs(num) / 60;
    if (minutes < 60) {
        return fmt::format("{0}{1:02d}m{2:02d}s", negative_sign, minutes, seconds);
    }

    // display [ -]HHhMMm
    int64_t hours = minutes / 60;
    minutes = minutes % 60;
    if (hours < 24) {
        return fmt::format("{0}{1:02d}h{2:02d}m", negative_sign, hours, minutes);
    }

    // display [ -]DDdHHh
    int64_t days = hours / 24;
    hours = hours % 24;
    if (days < 100) {
        return fmt::format("{0}{1:02d}d{2:02d}h", negative_sign, days, hours);
    }

    // display [ -]?
    return fmt::format("{0}?     ", negative_sign);
}


}  // namespace libdnf5::cli::progressbar
