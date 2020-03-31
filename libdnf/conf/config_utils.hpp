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

#ifndef _LIBDNF_CONFIG_PRIVATE_HPP
#define _LIBDNF_CONFIG_PRIVATE_HPP

#include "Option.hpp"

namespace libdnf {

template<typename T>
static void optionTListAppend(T & option, Option::Priority priority, const std::string & value)
{
    if (value.empty()) {
        option.set(priority, value);
        return;
    }
    auto addPriority = priority < option.getPriority() ? option.getPriority() : priority;
    auto val = option.fromString(value);
    bool first = true;
    for (auto & item : val) {
        if (item.empty()) {
            if (first) {
                option.set(priority, item);
            }
        } else {
            auto origValue = option.getValue();
            origValue.push_back(item);
            option.set(addPriority, origValue);
        }
        first = false;
    }
}

}

#endif
