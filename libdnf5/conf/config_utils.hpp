/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF5_CONF_CONFIG_PRIVATE_HPP
#define LIBDNF5_CONF_CONFIG_PRIVATE_HPP

#include "libdnf5/conf/option.hpp"

namespace libdnf5 {

template <typename T>
static void option_T_list_append(T & option, Option::Priority priority, const std::string & value) {
    if (value.empty()) {
        option.set(priority, value);
        return;
    }
    auto add_priority = priority < option.get_priority() ? option.get_priority() : priority;
    auto val = option.from_string(value);
    bool first = true;
    for (auto & item : val) {
        if (item.empty()) {
            if (first) {
                option.set(priority, item);
            }
        } else {
            auto orig_value = option.get_value();
            orig_value.insert(orig_value.end(), item);
            option.set(add_priority, orig_value);
        }
        first = false;
    }
}

}  // namespace libdnf5

#endif
