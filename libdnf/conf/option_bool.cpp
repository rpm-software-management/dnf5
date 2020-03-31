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

#include "OptionBool.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

namespace libdnf {

OptionBool::OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[])
: Option(Priority::DEFAULT), trueValues(trueVals), falseValues(falseVals)
, defaultValue(defaultValue), value(defaultValue) {}

OptionBool::OptionBool(bool defaultValue)
: OptionBool(defaultValue, nullptr, nullptr) {}

bool OptionBool::fromString(std::string value) const
{
    for (auto & ch : value)
        ch = std::tolower(ch);
    for (auto it = getFalseValues(); *it; ++it) {
        if (value == *it)
            return false;
    }
    for (auto it = getTrueValues(); *it; ++it) {
        if (value == *it)
            return true;
    }
    throw InvalidValue(tfm::format(_("invalid boolean value '%s'"), value));
}

void OptionBool::set(Priority priority, bool value)
{
    if (priority >= this->priority) {
        this->value = value;
        this->priority = priority;
    }
}

void OptionBool::set(Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}

std::string OptionBool::toString(bool value) const
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

}
