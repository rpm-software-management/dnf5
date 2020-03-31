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

#include "OptionNumber.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

#include <limits>

namespace libdnf {

template <typename T>
bool fromString(T & out, const std::string & in, std::ios_base & (*manipulator)(std::ios_base &))
{
   std::istringstream iss(in);
   return !(iss >> manipulator >> out).fail();
}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min, T max)
: Option(Priority::DEFAULT), defaultValue(defaultValue), min(min), max(max), value(defaultValue)
{
    test(defaultValue);
}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min)
: OptionNumber(defaultValue, min, std::numeric_limits<T>::max()) {}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue)
: OptionNumber(defaultValue, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()) {}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min, T max, FromStringFunc && fromStringFunc)
: Option(Priority::DEFAULT)
, fromStringUser(std::move(fromStringFunc))
, defaultValue(defaultValue), min(min), max(max), value(defaultValue)
{
    test(defaultValue);
}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, T min, FromStringFunc && fromStringFunc)
: OptionNumber(defaultValue, min, std::numeric_limits<T>::max(), std::move(fromStringFunc)) {}

template <typename T>
OptionNumber<T>::OptionNumber(T defaultValue, FromStringFunc && fromStringFunc)
: OptionNumber(defaultValue, std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), std::move(fromStringFunc)) {}

template <typename T>
void OptionNumber<T>::test(ValueType value) const
{
    if (value > max)
        throw InvalidValue(tfm::format(_("given value [%d] should be less than "
                                        "allowed value [%d]."), value, max));
    else if (value < min)
        throw InvalidValue(tfm::format(_("given value [%d] should be greater than "
                                        "allowed value [%d]."), value, min));
}

template <typename T>
T OptionNumber<T>::fromString(const std::string & value) const
{
    if (fromStringUser)
        return fromStringUser(value);
    ValueType val;
    if (libdnf::fromString<ValueType>(val, value, std::dec))
        return val;
    throw InvalidValue(_("invalid value"));
}

template <typename T>
void OptionNumber<T>::set(Priority priority, ValueType value)
{
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

template <typename T>
void OptionNumber<T>::set(Option::Priority priority, const std::string & value)
{
    set(priority, fromString(value));
}

template <typename T>
std::string OptionNumber<T>::toString(ValueType value) const
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template class OptionNumber<std::int32_t>;
template class OptionNumber<std::uint32_t>;
template class OptionNumber<std::int64_t>;
template class OptionNumber<std::uint64_t>;
template class OptionNumber<float>;

}
