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

#include "libdnf5/conf/option_seconds.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

namespace libdnf5 {

OptionSeconds::OptionSeconds(ValueType default_value, ValueType min, ValueType max)
    : OptionNumber(default_value, min, max) {}

OptionSeconds::OptionSeconds(ValueType default_value, ValueType min) : OptionNumber(default_value, min) {}

OptionSeconds::OptionSeconds(ValueType default_value) : OptionNumber(default_value, -1) {}

OptionSeconds::ValueType OptionSeconds::from_string(const std::string & value) const {
    constexpr int seconds_in_minute = 60;
    constexpr int minutes_in_hour = 60;
    constexpr int hours_in_day = 24;
    if (value.empty()) {
        throw OptionInvalidValueError(M_("Empty time option value"));
    }

    if (value == "-1" || value == "never") {  // Special cache timeout, meaning never
        return -1;
    }

    std::size_t idx;
    double res;
    try {
        res = std::stod(value, &idx);
    } catch (...) {
        throw OptionInvalidValueError(M_("Invalid time option value \"{}\", number or \"never\" expected"), value);
    }
    if (res < 0) {
        throw OptionInvalidValueError(
            M_("Invalid time option value \"{}\", negative values except \"-1\" not allowed"), value);
    }

    if (idx < value.length()) {
        if (idx < value.length() - 1) {
            throw OptionInvalidValueError(M_("Unknown time format \"{}\""), value);
        }
        switch (value.back()) {
            case 's':
            case 'S':
                break;
            case 'm':
            case 'M':
                res *= seconds_in_minute;
                break;
            case 'h':
            case 'H':
                res *= seconds_in_minute * minutes_in_hour;
                break;
            case 'd':
            case 'D':
                res *= seconds_in_minute * minutes_in_hour * hours_in_day;
                break;
            default:
                throw OptionInvalidValueError(M_("Unknown time unit '{}'"), std::string(&value.back(), 1));
        }
    }

    return static_cast<ValueType>(res);
}

void OptionSeconds::set(Priority priority, const std::string & value) {
    set(priority, from_string(value));
}

void OptionSeconds::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

}  // namespace libdnf5
