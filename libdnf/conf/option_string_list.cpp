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

#include "OptionStringList.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"
#include "regex/regex.hpp"

namespace libdnf {

OptionStringList::OptionStringList(const ValueType & defaultValue)
: Option(Priority::DEFAULT), defaultValue(defaultValue), value(defaultValue) {}

OptionStringList::OptionStringList(const ValueType & defaultValue, const std::string & regex, bool icase)
: Option(Priority::DEFAULT), regex(regex), icase(icase), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

OptionStringList::OptionStringList(const std::string & defaultValue)
: Option(Priority::DEFAULT)
{
    this->value = this->defaultValue = fromString(defaultValue);
}

OptionStringList::OptionStringList(const std::string & defaultValue, const std::string & regex, bool icase)
: Option(Priority::DEFAULT), regex(regex), icase(icase)
{
    this->defaultValue = fromString(defaultValue);
    test(this->defaultValue);
    value = this->defaultValue;
}

void OptionStringList::test(const std::vector<std::string> & value) const
{
    if (regex.empty())
        return;
    Regex regexObj(regex.c_str(), (icase ? REG_ICASE : 0) | REG_EXTENDED | REG_NOSUB);
    for (const auto & val : value) {
        if (!regexObj.match(val.c_str()))
            throw InvalidValue(tfm::format(_("'%s' is not an allowed value"), val));
    }
}

OptionStringList::ValueType OptionStringList::fromString(const std::string & value) const
{
    std::vector<std::string> tmp;
    auto start = value.find_first_not_of(" ");
    while (start != value.npos && start < value.length()) {
        auto end = value.find_first_of(" ,\n", start);
        if (end == value.npos) {
            tmp.push_back(value.substr(start));
            break;
        }
        tmp.push_back(value.substr(start, end - start));
        start = value.find_first_not_of(" ", end + 1);
        if (start != value.npos && value[start] == ',' && value[end] == ' ') {
            end = start;
            start = value.find_first_not_of(" ", start + 1);
        }
        if (start != value.npos && value[start] == '\n' && (value[end] == ' ' || value[end] == ','))
            start = value.find_first_not_of(" ", start + 1);
    }
    return tmp;
}

void OptionStringList::set(Priority priority, const ValueType & value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

void OptionStringList::set(Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}

std::string OptionStringList::toString(const ValueType & value) const
{
    std::ostringstream oss;
    bool next{false};
    for (auto & val : value) {
        if (next)
            oss << ", ";
        else
            next = true;
        oss << val;
    }
    return oss.str();
}

}
