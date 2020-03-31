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

#include "OptionEnum.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

#include <sstream>

namespace libdnf {

template <typename T>
bool fromString(T & out, const std::string & in, std::ios_base & (*manipulator)(std::ios_base &))
{
   std::istringstream iss(in);
   return !(iss >> manipulator >> out).fail();
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals)
: Option(Priority::DEFAULT), enumVals(enumVals), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals)
: Option(Priority::DEFAULT), enumVals(std::move(enumVals)), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc)
: Option(Priority::DEFAULT), fromStringUser(std::move(fromStringFunc))
, enumVals(enumVals), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc)
: Option(Priority::DEFAULT), fromStringUser(std::move(fromStringFunc))
, enumVals(std::move(enumVals)), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

template <typename T>
void OptionEnum<T>::test(ValueType value) const
{
    auto it = std::find(enumVals.begin(), enumVals.end(), value);
    if (it == enumVals.end())
        throw InvalidValue(tfm::format(_("'%s' is not an allowed value"), value));
}

template <typename T>
T OptionEnum<T>::fromString(const std::string & value) const
{
    if (fromStringUser)
        return fromStringUser(value);
    T val;
    if (libdnf::fromString<ValueType>(val, value, std::dec))
        return val;
    throw InvalidValue(_("invalid value"));
}

template <typename T>
void OptionEnum<T>::set(Priority priority, ValueType value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

template <typename T>
void OptionEnum<T>::set(Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}

template <typename T>
T OptionEnum<T>::getValue() const
{
    return value;
}

template <typename T>
T OptionEnum<T>::getDefaultValue() const
{
    return defaultValue;
}

template <typename T>
std::string OptionEnum<T>::toString(ValueType value) const
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template <typename T>
std::string OptionEnum<T>::getValueString() const 
{
    return toString(value);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals)
: Option(Priority::DEFAULT), enumVals(enumVals), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals)
: Option(Priority::DEFAULT), enumVals(std::move(enumVals)), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc)
: Option(Priority::DEFAULT), fromStringUser(std::move(fromStringFunc))
, enumVals(enumVals), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

OptionEnum<std::string>::OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc)
: Option(Priority::DEFAULT), fromStringUser(std::move(fromStringFunc))
, enumVals(std::move(enumVals)), defaultValue(defaultValue), value(defaultValue)
{
    test(defaultValue);
}

void OptionEnum<std::string>::test(const std::string & value) const
{
    auto it = std::find(enumVals.begin(), enumVals.end(), value);
    if (it == enumVals.end())
        throw InvalidValue(tfm::format(_("'%s' is not an allowed value"), value));
}

std::string OptionEnum<std::string>::fromString(const std::string & value) const
{
    if (fromStringUser)
        return fromStringUser(value);
    return value;
}

void OptionEnum<std::string>::set(Priority priority, const std::string & value)
{
    auto val = fromString(value);
    if (priority >= this->priority) {
        test(val);
        this->value = val;
        this->priority = priority;
    }
}

}
