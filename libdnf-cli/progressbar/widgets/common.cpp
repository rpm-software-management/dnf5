/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "common.hpp"

#include <fmt/format.h>


namespace libdnf::cli::progressbar {


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


std::string format_size(int64_t num) {
    auto i = static_cast<float>(num);
    int index = 0;
    while (i > 999) {
        i /= 1024;
        index++;
    }
    return fmt::format("{0:5.1f} {1:>3s}", i, SIZE_UNITS[index]);
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


}  // namespace libdnf::cli::progressbar
