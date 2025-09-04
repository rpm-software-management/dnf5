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

#include "option_string_private.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <regex>

namespace libdnf5 {

OptionString * OptionString::clone() const {
    return new OptionString(*this);
}

const std::string & OptionString::get_default_value() const noexcept {
    return p_impl->default_value;
}

std::string OptionString::get_value_string() const {
    return get_value();
}

std::string OptionString::from_string(const std::string & value) const {
    return value;
}


OptionString::OptionString(std::string default_value)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value))) {}

OptionString::OptionString(const char * default_value) : p_impl(new Impl()) {
    if (default_value) {
        p_impl->value = p_impl->default_value = default_value;
        set_priority(Priority::DEFAULT);
    }
}

OptionString::OptionString(std::string default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value), std::move(regex), icase)) {
    test(p_impl->value);
}

OptionString::OptionString(const char * default_value, std::string regex, bool icase)
    : p_impl(new Impl(std::move(regex), icase)) {
    if (default_value) {
        p_impl->default_value = default_value;
        test(p_impl->default_value);
        p_impl->value = p_impl->default_value;
        set_priority(Priority::DEFAULT);
    }
}

OptionString::~OptionString() = default;

OptionString::OptionString(const OptionString & src) = default;

OptionString & OptionString::operator=(const OptionString & other) = default;

void OptionString::test(const std::string & value) const {
    if (p_impl->regex.empty()) {
        return;
    }

    auto flags = std::regex::ECMAScript | std::regex::nosubs;
    if (p_impl->icase) {
        flags |= std::regex::icase;
    }
    if (!std::regex_match(value, std::regex(p_impl->regex, flags))) {
        throw OptionValueNotAllowedError(
            M_("Input value \"{}\" not allowed, allowed values for this option are defined by regular expression "
               "\"{}\""),
            value,
            p_impl->regex);
    }
}

void OptionString::set(Priority priority, const std::string & value) {
    assert_not_locked();

    if (priority >= get_priority()) {
        test(value);
        p_impl->value = value;
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
    return p_impl->value;
}

}  // namespace libdnf5
