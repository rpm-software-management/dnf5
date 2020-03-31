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

#ifndef _LIBDNF_OPTION_SECONDS_HPP
#define _LIBDNF_OPTION_SECONDS_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "OptionNumber.hpp"

namespace libdnf {

/**
* @class OptionSeconds
*
* @brief An option representing an integer value of seconds.
*
* Valid inputs: 100, 1.5m, 90s, 1.2d, 1d, 0xF, 0.1, -1, never.
* Invalid inputs: -10, -0.1, 45.6Z, 1d6h, 1day, 1y.
*/
class OptionSeconds : public OptionNumber<std::int32_t> {
public:
    OptionSeconds(ValueType defaultValue, ValueType min, ValueType max);
    OptionSeconds(ValueType defaultValue, ValueType min);
    OptionSeconds(ValueType defaultValue);
    OptionSeconds * clone() const override;
    ValueType fromString(const std::string & value) const;
    using OptionNumber<std::int32_t>::set;
    void set(Priority priority, const std::string & value) override;
};

inline OptionSeconds * OptionSeconds::clone() const
{
    return new OptionSeconds(*this);
}

}

#endif

#endif
