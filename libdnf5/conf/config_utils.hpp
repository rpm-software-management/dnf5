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
    if (priority < option.get_priority()) {
        return;
    }
    if (value.empty()) {
        option.set(priority, value);
        return;
    }
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
            option.set(priority, orig_value);
        }
        first = false;
    }
}


/// @brief Replaces globs (like /etc/foo.d/\\*.foo) by content of matching files.
///
/// Ignores comment lines (start with '#') and blank lines in files.
/// Result:
/// Words delimited by spaces. Characters ',' and '\n' are replaced by spaces.
/// Extra spaces are removed.
/// @param strWithGlobs Input string with globs
/// @return Words delimited by space
std::string resolve_path_globs(const std::string & str_with_globs, const std::filesystem::path & installroot);


}  // namespace libdnf5

#endif
