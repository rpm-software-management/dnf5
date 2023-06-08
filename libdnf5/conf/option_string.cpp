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

#include "libdnf5/conf/option_string.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"

#include <regex>

namespace libdnf5 {

OptionString::OptionString(const std::string & default_value)
    : Option(Priority::DEFAULT),
      icase(false),
      default_value(default_value),
      value(default_value) {}

OptionString::OptionString(const char * default_value) : icase(false) {
    if (default_value) {
        this->value = this->default_value = default_value;
        set_priority(Priority::DEFAULT);
    }
}

OptionString::OptionString(const std::string & default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      regex(std::move(regex)),
      icase(icase),
      default_value(default_value),
      value(default_value) {
    test(default_value);
}

OptionString::OptionString(const char * default_value, std::string regex, bool icase)
    : regex(std::move(regex)),
      icase(icase) {
    if (default_value) {
        this->default_value = default_value;
        test(this->default_value);
        this->value = this->default_value;
        set_priority(Priority::DEFAULT);
    }
}

void OptionString::test(const std::string & value) const {
    if (regex.empty()) {
        return;
    }

    auto flags = std::regex::ECMAScript | std::regex::nosubs;
    if (icase) {
        flags |= std::regex::icase;
    }
    if (!std::regex_match(value, std::regex(regex, flags))) {
        throw OptionValueNotAllowedError(
            M_("Input value \"{}\" not allowed, allowed values for this option are defined by regular expression "
               "\"{}\""),
            value,
            regex);
    }
}

void OptionString::set(Priority priority, const std::string & value) {
    assert_not_locked();

    if (priority >= get_priority()) {
        test(value);
        this->value = value;
        set_priority(priority);
    }
}

void OptionString::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

const std::string & OptionString::get_value() const {
    if (get_priority() == Priority::EMPTY) {
        //TODO(jrohel): We don't know the option name at this time. Add a text name to the options
        //              or extend exception (add name) in upper layer.
        throw OptionValueNotSetError(M_("Option value is not set"));
    }
    return value;
}

}  // namespace libdnf5
