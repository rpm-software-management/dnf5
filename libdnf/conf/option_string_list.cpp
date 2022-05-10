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

#include "libdnf/conf/option_string_list.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"

#include <regex>

namespace libdnf {

OptionStringList::OptionStringList(const ValueType & default_value)
    : Option(Priority::DEFAULT),
      icase(false),
      default_value(default_value),
      value(default_value) {}

OptionStringList::OptionStringList(const ValueType & default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      regex(std::move(regex)),
      icase(icase),
      default_value(default_value),
      value(default_value) {
    test(default_value);
}

OptionStringList::OptionStringList(const std::string & default_value) : Option(Priority::DEFAULT), icase(false) {
    this->value = this->default_value = from_string(default_value);
}

OptionStringList::OptionStringList(const std::string & default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      regex(std::move(regex)),
      icase(icase) {
    this->default_value = from_string(default_value);
    test(this->default_value);
    value = this->default_value;
}

void OptionStringList::test(const std::vector<std::string> & value) const {
    if (regex.empty()) {
        return;
    }
    std::regex re(
        regex,
        std::regex::nosubs | std::regex::extended | (icase ? std::regex::icase : std::regex_constants::ECMAScript));
    for (const auto & val : value) {
        if (!std::regex_match(val, re)) {
            throw OptionValueNotAllowedError(
                M_("Input value \"{}\" not allowed, allowed values for this option are defined by regular expression "
                   "\"{}\""),
                val,
                regex);
        }
    }
}

OptionStringList::ValueType OptionStringList::from_string(const std::string & value) const {
    std::vector<std::string> tmp;
    auto start = value.find_first_not_of(' ');
    while (start != std::string::npos && start < value.length()) {
        auto end = value.find_first_of(" ,\n", start);
        if (end == std::string::npos) {
            tmp.push_back(value.substr(start));
            break;
        }
        tmp.push_back(value.substr(start, end - start));
        start = value.find_first_not_of(' ', end + 1);
        if (start != std::string::npos && value[start] == ',' && value[end] == ' ') {
            end = start;
            start = value.find_first_not_of(' ', start + 1);
        }
        if (start != std::string::npos && value[start] == '\n' && (value[end] == ' ' || value[end] == ',')) {
            start = value.find_first_not_of(' ', start + 1);
        }
    }
    return tmp;
}

void OptionStringList::set(Priority priority, const ValueType & value) {
    assert_not_locked();

    if (priority >= get_priority()) {
        test(value);
        this->value = value;
        set_priority(priority);
    }
}

void OptionStringList::set(Priority priority, const std::string & value) {
    set(priority, from_string(value));
}

std::string OptionStringList::to_string(const ValueType & value) const {
    std::ostringstream oss;
    bool next{false};
    for (auto & val : value) {
        if (next) {
            oss << ", ";
        } else {
            next = true;
        }
        oss << val;
    }
    return oss.str();
}

}  // namespace libdnf
