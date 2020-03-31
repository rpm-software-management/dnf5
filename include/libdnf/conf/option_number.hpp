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

#ifndef _LIBDNF_OPTION_NUMBER_HPP
#define _LIBDNF_OPTION_NUMBER_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Option.hpp"

#include <functional>

namespace libdnf {

template <typename T>
class OptionNumber : public Option {
public:
    typedef T ValueType;
    typedef std::function<ValueType (const std::string &)> FromStringFunc;

    OptionNumber(T defaultValue, T min, T max);
    OptionNumber(T defaultValue, T min);
    OptionNumber(T defaultValue);
    OptionNumber(T defaultValue, T min, T max, FromStringFunc && fromStringFunc);
    OptionNumber(T defaultValue, T min, FromStringFunc && fromStringFunc);
    OptionNumber(T defaultValue, FromStringFunc && fromStringFunc);
    OptionNumber * clone() const override;
    void test(ValueType value) const;
    T fromString(const std::string & value) const;
    void set(Priority priority, ValueType value);
    void set(Priority priority, const std::string & value) override;
    T getValue() const;
    T getDefaultValue() const;
    std::string toString(ValueType value) const;
    std::string getValueString() const override;

protected:
    FromStringFunc fromStringUser;
    ValueType defaultValue;
    ValueType min;
    ValueType max;
    ValueType value;
};

template <typename T>
inline OptionNumber<T> * OptionNumber<T>::clone() const
{
    return new OptionNumber<T>(*this);
}

template <typename T>
inline T OptionNumber<T>::getValue() const
{
    return value;
}

template <typename T>
inline T OptionNumber<T>::getDefaultValue() const
{
    return defaultValue;
}

template <typename T>
inline std::string OptionNumber<T>::getValueString() const
{
    return toString(value); 
}

extern template class OptionNumber<std::int32_t>;
extern template class OptionNumber<std::uint32_t>;
extern template class OptionNumber<std::int64_t>;
extern template class OptionNumber<std::uint64_t>;
extern template class OptionNumber<float>;

}

#endif

#endif
