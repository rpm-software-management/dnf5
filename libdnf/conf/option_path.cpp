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

#include "OptionPath.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace libdnf {

static std::string removeFileProt(const std::string & value)
{
    if (value.compare(0, 7, "file://") == 0)
        return value.substr(7);
    return value;
}

OptionPath::OptionPath(const std::string & defaultValue, bool exists, bool absPath)
: OptionString(defaultValue), exists(exists), absPath(absPath)
{
    this->defaultValue = removeFileProt(this->defaultValue);
    test(this->defaultValue);
    this->value = this->defaultValue;
}

OptionPath::OptionPath(const char * defaultValue, bool exists, bool absPath)
: OptionString(defaultValue), exists(exists), absPath(absPath)
{
    if (defaultValue) {
        this->defaultValue = removeFileProt(this->defaultValue);
        test(this->defaultValue);
        this->value = this->defaultValue;
    }
}

OptionPath::OptionPath(const std::string & defaultValue, const std::string & regex, bool icase, bool exists, bool absPath)
: OptionString(removeFileProt(defaultValue), regex, icase), exists(exists), absPath(absPath)
{
    this->defaultValue = removeFileProt(this->defaultValue);
    test(this->defaultValue);
    this->value = this->defaultValue;
}

OptionPath::OptionPath(const char * defaultValue, const std::string & regex, bool icase, bool exists, bool absPath)
: OptionString(defaultValue, regex, icase), exists(exists), absPath(absPath)
{
    if (defaultValue) {
        this->defaultValue = removeFileProt(this->defaultValue);
        test(this->defaultValue);
        this->value = this->defaultValue;
    }
}

void OptionPath::test(const std::string & value) const
{
    if (absPath && value[0] != '/')
        throw InvalidValue(tfm::format(_("given path '%s' is not absolute."), value));

    struct stat buffer; 
    if (exists && stat(value.c_str(), &buffer))
        throw InvalidValue(tfm::format(_("given path '%s' does not exist."), value));
}

void OptionPath::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority) {
        OptionString::test(value);
        auto val = removeFileProt(value);
        test(val);
        this->value = val;
        this->priority = priority;
    }
}

}
