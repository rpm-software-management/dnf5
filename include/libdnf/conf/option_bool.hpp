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

#ifndef _LIBDNF_OPTION_BOOL_HPP
#define _LIBDNF_OPTION_BOOL_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Option.hpp"

namespace libdnf {

constexpr const char * defTrueValues[]{"1", "yes", "true", "on", nullptr};
constexpr const char * defFalseValues[]{"0", "no", "false", "off", nullptr};

class OptionBool : public Option {
public:
    typedef bool ValueType;

    OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[]);
    OptionBool(bool defaultValue);
    OptionBool * clone() const override;
    void test(bool) const;
    bool fromString(std::string value) const;
    void set(Priority priority, bool value);
    void set(Priority priority, const std::string & value) override;
    bool getValue() const noexcept;
    bool getDefaultValue() const noexcept;
    std::string toString(bool value) const;
    std::string getValueString() const override;
    const char * const * getTrueValues() const noexcept;
    const char * const * getFalseValues() const noexcept;

protected:
    const char * const * const trueValues;
    const char * const * const falseValues;
    bool defaultValue;
    bool value;
};

inline OptionBool * OptionBool::clone() const
{
    return new OptionBool(*this);
}

inline void OptionBool::test(bool) const {}

inline bool OptionBool::getValue() const noexcept
{
    return value;
}

inline bool OptionBool::getDefaultValue() const noexcept
{
    return defaultValue;
}

inline std::string OptionBool::getValueString() const
{
    return toString(value);
}

inline const char * const * OptionBool::getTrueValues() const noexcept
{
    return trueValues ? trueValues : defTrueValues;
}

inline const char * const * OptionBool::getFalseValues() const noexcept
{
    return falseValues ? falseValues : defFalseValues; 
}

}

#endif

#endif
