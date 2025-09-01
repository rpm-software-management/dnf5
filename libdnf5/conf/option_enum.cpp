// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/conf/option_enum.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <algorithm>

namespace libdnf5 {

class OptionEnum::Impl {
public:
    Impl(std::string && default_value, std::vector<std::string> && enum_vals)
        : enum_vals(std::move(enum_vals)),
          default_value(default_value),
          value(std::move(default_value)) {}

    Impl(std::string && default_value, std::vector<std::string> && enum_vals, FromStringFunc && from_string_func)
        : from_string_user(std::move(from_string_func)),
          enum_vals(std::move(enum_vals)),
          default_value(default_value),
          value(std::move(default_value)) {}

private:
    friend OptionEnum;

    FromStringFunc from_string_user;
    std::vector<std::string> enum_vals;
    std::string default_value;
    std::string value;
};

OptionEnum::OptionEnum(std::string default_value, std::vector<std::string> enum_vals)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value), std::move(enum_vals))) {
    test(p_impl->value);
}

OptionEnum::OptionEnum(
    std::string default_value, std::vector<std::string> enum_vals, FromStringFunc && from_string_func)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value), std::move(enum_vals), std::move(from_string_func))) {
    test(p_impl->value);
}

OptionEnum::~OptionEnum() = default;

void OptionEnum::test(const std::string & value) const {
    auto it = std::find(p_impl->enum_vals.begin(), p_impl->enum_vals.end(), value);
    if (it == p_impl->enum_vals.end()) {
        throw OptionValueNotAllowedError(M_("Enum option value \"{}\" not allowed"), value);
    }
}

std::string OptionEnum::from_string(const std::string & value) const {
    if (p_impl->from_string_user) {
        return p_impl->from_string_user(value);
    }
    return value;
}

void OptionEnum::set(Priority priority, const std::string & value) {
    assert_not_locked();

    auto val = from_string(value);
    if (priority >= get_priority()) {
        test(val);
        p_impl->value = val;
        set_priority(priority);
    }
}

void OptionEnum::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

OptionEnum * OptionEnum::clone() const {
    return new OptionEnum(*this);
}

const std::string & OptionEnum::get_value() const {
    return p_impl->value;
}

const std::string & OptionEnum::get_default_value() const {
    return p_impl->default_value;
}

std::string OptionEnum::get_value_string() const {
    return p_impl->value;
}

}  // namespace libdnf5
