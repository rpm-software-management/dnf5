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

#include "OptionBinds.hpp"

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

#include <utility>

namespace libdnf {

// ========== OptionBinds::Item class ===============

OptionBinds::Item::Item(Option & option, const NewStringFunc & newString,
    const GetValueStringFunc & getValueString, bool addValue)
: option(&option), newStr(newString), getValueStr(getValueString), addValue(addValue) {}

OptionBinds::Item::Item(Option & option, NewStringFunc && newString,
    GetValueStringFunc && getValueString, bool addValue)
: option(&option), newStr(std::move(newString)), getValueStr(std::move(getValueString)), addValue(addValue) {}

OptionBinds::Item::Item(Option & option)
: option(&option) {}

Option::Priority OptionBinds::Item::getPriority() const
{
    return option->getPriority();
}

void OptionBinds::Item::newString(Option::Priority priority, const std::string & value)
{
    if (newStr)
        newStr(priority, value);
    else
        option->set(priority, value);
}

std::string OptionBinds::Item::getValueString() const
{
    if (getValueStr)
        return getValueStr();
    else
        return option->getValueString();
}

bool OptionBinds::Item::getAddValue() const
{
    return addValue;
}


// =========== OptionBinds class ===============

const char * OptionBinds::OutOfRange::what() const noexcept
{
    try {
        if (tmpMsg.empty())
            tmpMsg = tfm::format(_("Configuration: OptionBinding with id \"%s\" does not exist"),
                Exception::what());
        return tmpMsg.c_str();
    } catch (...) {
        return Exception::what();
    }
}

const char * OptionBinds::AlreadyExists::what() const noexcept
{
    try {
        if (tmpMsg.empty())
            tmpMsg = tfm::format(_("Configuration: OptionBinding with id \"%s\" already exists"),
                Exception::what());
        return tmpMsg.c_str();
    } catch (...) {
        return Exception::what();
    }
}

OptionBinds::Item & OptionBinds::at(const std::string & id)
{
    auto item = items.find(id);
    if (item == items.end())
        throw OutOfRange(id);
    return item->second;
}

const OptionBinds::Item & OptionBinds::at(const std::string & id) const
{
    auto item = items.find(id);
    if (item == items.end())
        throw OutOfRange(id);
    return item->second;
}

OptionBinds::Item & OptionBinds::add(const std::string & id, Option & option,
    const Item::NewStringFunc & newString, const Item::GetValueStringFunc & getValueString, bool addValue)
{
    auto item = items.find(id);
    if (item != items.end())
        throw AlreadyExists(id);
    auto res = items.emplace(id, Item(option, newString, getValueString, addValue));
    return res.first->second;
}

OptionBinds::Item & OptionBinds::add(const std::string & id, Option & option,
    Item::NewStringFunc && newString, Item::GetValueStringFunc && getValueString, bool addValue)
{
    auto item = items.find(id);
    if (item != items.end())
        throw AlreadyExists(id);
    auto res = items.emplace(id, Item(option, std::move(newString), std::move(getValueString), addValue));
    return res.first->second;
}

OptionBinds::Item & OptionBinds::add(const std::string & id, Option & option)
{
    auto item = items.find(id);
    if (item != items.end())
        throw AlreadyExists(id);
    auto res = items.emplace(id, Item(option));
    return res.first->second;
}

}
