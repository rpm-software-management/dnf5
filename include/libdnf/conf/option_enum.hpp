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

#ifndef _LIBDNF_OPTION_ENUM_HPP
#define _LIBDNF_OPTION_ENUM_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Option.hpp"

#include <functional>
#include <vector>

namespace libdnf {

template <typename T>
class OptionEnum : public Option {
public:
    typedef T ValueType;
    typedef std::function<ValueType (const std::string &)> FromStringFunc;

    OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals);
    OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals);
    OptionEnum(ValueType defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc);
    OptionEnum(ValueType defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc);
    OptionEnum * clone() const override;
    void test(ValueType value) const;
    ValueType fromString(const std::string & value) const;
    void set(Priority priority, ValueType value);
    void set(Priority priority, const std::string & value) override;
    T getValue() const;
    T getDefaultValue() const;
    std::string toString(ValueType value) const;
    std::string getValueString() const override;

protected:
    FromStringFunc fromStringUser;
    std::vector<ValueType> enumVals;
    ValueType defaultValue;
    ValueType value;
};

template <>
class OptionEnum<std::string> : public Option {
public:
    typedef std::string ValueType;
    typedef std::function<ValueType (const std::string &)> FromStringFunc;

    OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals);
    OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals);
    OptionEnum(const std::string & defaultValue, const std::vector<ValueType> & enumVals, FromStringFunc && fromStringFunc);
    OptionEnum(const std::string & defaultValue, std::vector<ValueType> && enumVals, FromStringFunc && fromStringFunc);
    OptionEnum * clone() const override;
    void test(const std::string & value) const;
    std::string fromString(const std::string & value) const;
    void set(Priority priority, const std::string & value) override;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const;
    std::string getValueString() const override;

protected:
    FromStringFunc fromStringUser;
    std::vector<ValueType> enumVals;
    ValueType defaultValue;
    ValueType value;
};

template <typename T>
inline OptionEnum<T> * OptionEnum<T>::clone() const
{
    return new OptionEnum<T>(*this);
}

inline OptionEnum<std::string> * OptionEnum<std::string>::clone() const
{
    return new OptionEnum<std::string>(*this);
}

inline const std::string & OptionEnum<std::string>::getValue() const
{
    return value;
}

inline const std::string & OptionEnum<std::string>::getDefaultValue() const
{
    return defaultValue;
}

inline std::string OptionEnum<std::string>::getValueString() const
{
    return value;
}

}

#endif

#endif
