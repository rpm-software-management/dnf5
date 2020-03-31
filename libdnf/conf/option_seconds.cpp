/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "OptionSeconds.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

namespace libdnf {

OptionSeconds::OptionSeconds(ValueType defaultValue, ValueType min, ValueType max)
: OptionNumber(defaultValue, min, max) {}

OptionSeconds::OptionSeconds(ValueType defaultValue, ValueType min)
: OptionNumber(defaultValue, min) {}

OptionSeconds::OptionSeconds(ValueType defaultValue)
: OptionNumber(defaultValue) {}

OptionSeconds::ValueType OptionSeconds::fromString(const std::string & value) const
{
    if (value.empty())
        throw InvalidValue(_("no value specified"));

    if (value == "-1" || value == "never") // Special cache timeout, meaning never
        return -1;

    std::size_t idx;
    auto res = std::stod(value, &idx);
    if (res < 0)
        throw InvalidValue(tfm::format(_("seconds value '%s' must not be negative"), value));

    if (idx < value.length()) {
        if (idx < value.length() - 1)
            throw InvalidValue(tfm::format(_("could not convert '%s' to seconds"), value));
        switch (value.back()) {
            case 's': case 'S':
                break;
            case 'm': case 'M':
                res *= 60;
                break;
            case 'h': case 'H':
                res *= 60 * 60;
                break;
            case 'd': case 'D':
                res *= 60 * 60 * 24;
                break;
            default:
                throw InvalidValue(tfm::format(_("unknown unit '%s'"), value.back()));
        }
    }

    return res;
}

void OptionSeconds::set(Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}

}
