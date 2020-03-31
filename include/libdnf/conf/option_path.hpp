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

#ifndef _LIBDNF_OPTION_PATH_HPP
#define _LIBDNF_OPTION_PATH_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "OptionString.hpp"

namespace libdnf {

/**
* @class OptionPath
*
* @brief Option for file path which can validate path existence.
*
*/
class OptionPath : public OptionString {
public:
    OptionPath(const std::string & defaultValue, bool exists = false, bool absPath = false);
    OptionPath(const char * defaultValue, bool exists = false, bool absPath = false);
    OptionPath(const std::string & defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false);
    OptionPath(const char * defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false);
    OptionPath * clone() const override;
    void test(const std::string & value) const;
    void set(Priority priority, const std::string & value) override;

protected:
    bool exists;
    bool absPath;
};

inline OptionPath * OptionPath::clone() const
{
    return new OptionPath(*this);
}

}

#endif

#endif
