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

#include "OptionString.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"
#include "regex/regex.hpp"

namespace libdnf {

OptionString::OptionString(const std::string & defaultValue)
: Option(Priority::DEFAULT), defaultValue(defaultValue), value(defaultValue) {}

OptionString::OptionString(const char * defaultValue)
{
    if (defaultValue) {
        this->value = this->defaultValue = defaultValue;
        this->priority = Priority::DEFAULT;
    }
}

OptionString::OptionString(const std::string & defaultValue, const std::string & regex, bool icase)
: Option(Priority::DEFAULT), regex(regex), icase(icase), defaultValue(defaultValue), value(defaultValue) { test(defaultValue); }

OptionString::OptionString(const char * defaultValue, const std::string & regex, bool icase)
: regex(regex), icase(icase)
{
    if (defaultValue) {
        this->defaultValue = defaultValue;
        test(this->defaultValue);
        this->value = this->defaultValue;
        this->priority = Priority::DEFAULT;
    }
}

void OptionString::test(const std::string & value) const
{
    if (regex.empty())
        return;
    if (!Regex(regex.c_str(), (icase ? REG_ICASE : 0) | REG_EXTENDED | REG_NOSUB).match(value.c_str()))
        throw InvalidValue(tfm::format(_("'%s' is not an allowed value"), value));
}

void OptionString::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

const std::string & OptionString::getValue() const
{
    if (priority == Priority::EMPTY)
        throw ValueNotSet(_("GetValue(): Value not set"));
    return value;
}

}
