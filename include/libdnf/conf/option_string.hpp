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

#ifndef _LIBDNF_OPTION_STRING_HPP
#define _LIBDNF_OPTION_STRING_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Option.hpp"

namespace libdnf {

class OptionString : public Option {
public:
    typedef std::string ValueType;

    OptionString(const std::string & defaultValue);
    OptionString(const char * defaultValue);
    OptionString(const std::string & defaultValue, const std::string & regex, bool icase);
    OptionString(const char * defaultValue, const std::string & regex, bool icase);
    OptionString * clone() const override;
    void test(const std::string & value) const;
    void set(Priority priority, const std::string & value) override;
    std::string fromString(const std::string & value) const;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const noexcept;
    std::string getValueString() const override;

protected:
    std::string regex;
    bool icase;
    std::string defaultValue;
    std::string value;
};

inline OptionString * OptionString::clone() const
{
    return new OptionString(*this);
}

inline const std::string & OptionString::getDefaultValue() const noexcept
{
    return defaultValue;
}

inline std::string OptionString::getValueString() const
{
    return getValue();
}

inline std::string OptionString::fromString(const std::string & value) const
{
    return value;
}

}

#endif

#endif
