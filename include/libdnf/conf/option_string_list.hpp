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

#ifndef _LIBDNF_OPTION_STRING_LIST_HPP
#define _LIBDNF_OPTION_STRING_LIST_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Option.hpp"
#include <vector>

namespace libdnf {

class OptionStringList : public Option {
public:
    typedef std::vector<std::string> ValueType;

    OptionStringList(const ValueType & defaultValue);
    OptionStringList(const ValueType & defaultValue, const std::string & regex, bool icase);
    OptionStringList(const std::string & defaultValue);
    OptionStringList(const std::string & defaultValue, const std::string & regex, bool icase);
    OptionStringList * clone() const override;
    void test(const std::vector<std::string> & value) const;
    ValueType fromString(const std::string & value) const;
    virtual void set(Priority priority, const ValueType & value);
    void set(Priority priority, const std::string & value) override;
    const ValueType & getValue() const;
    const ValueType & getDefaultValue() const;
    std::string toString(const ValueType & value) const;
    std::string getValueString() const override;

protected:
    std::string regex;
    bool icase;
    ValueType defaultValue;
    ValueType value;
};

inline OptionStringList * OptionStringList::clone() const
{
    return new OptionStringList(*this);
}

inline const OptionStringList::ValueType & OptionStringList::getValue() const
{
    return value;
}

inline const OptionStringList::ValueType & OptionStringList::getDefaultValue() const
{
    return defaultValue;
}

inline std::string OptionStringList::getValueString() const
{
    return toString(value); 
}

}

#endif

#endif
