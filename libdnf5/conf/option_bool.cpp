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

#include "libdnf5/conf/option_bool.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <sstream>

namespace libdnf5 {

class OptionBool::Impl {
public:
    Impl(bool default_value, std::vector<std::string> && true_vals, std::vector<std::string> && false_vals)
        : true_values(std::make_unique<std::vector<std::string>>(std::move(true_vals))),
          false_values(std::make_unique<std::vector<std::string>>(std::move(false_vals))),
          default_value(default_value),
          value(default_value) {}

    Impl(bool default_value) : default_value(default_value), value(default_value) {}

    Impl(const OptionBool::Impl & src) : default_value(src.default_value), value(src.value) {
        if (src.true_values) {
            true_values = std::make_unique<std::vector<std::string>>(*src.true_values);
        }
        if (src.false_values) {
            false_values = std::make_unique<std::vector<std::string>>(*src.false_values);
        }
    }

    Impl & operator=(const OptionBool::Impl & other) {
        default_value = other.default_value;
        value = other.value;
        if (other.true_values) {
            true_values = std::make_unique<std::vector<std::string>>(*other.true_values);
        }
        if (other.false_values) {
            false_values = std::make_unique<std::vector<std::string>>(*other.false_values);
        }
        return *this;
    }

private:
    friend OptionBool;

    std::unique_ptr<std::vector<std::string>> true_values;
    std::unique_ptr<std::vector<std::string>> false_values;
    bool default_value;
    bool value;
};

OptionBool::OptionBool(const OptionBool & src) = default;

OptionBool::OptionBool(bool default_value, std::vector<std::string> true_vals, std::vector<std::string> false_vals)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(default_value, std::move(true_vals), std::move(false_vals))) {}

OptionBool::OptionBool(bool default_value) : Option(Priority::DEFAULT), p_impl(new Impl(default_value)) {}

OptionBool::~OptionBool() = default;

OptionBool & OptionBool::operator=(const OptionBool & other) = default;

bool OptionBool::from_string(const std::string & value) const {
    auto tmp_value = value;
    // Case insensitive conversion. Convert input value to lower case first.
    for (auto & ch : tmp_value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    for (auto & false_value : get_false_values()) {
        if (tmp_value == false_value) {
            return false;
        }
    }
    for (auto & true_value : get_true_values()) {
        if (tmp_value == true_value) {
            return true;
        }
    }
    throw OptionInvalidValueError(M_("Invalid boolean value \"{}\""), value);
}

void OptionBool::set(Priority priority, bool value) {
    assert_not_locked();

    if (priority >= get_priority()) {
        p_impl->value = value;
        set_priority(priority);
    }
}

void OptionBool::set(bool value) {
    set(Priority::RUNTIME, value);
}

void OptionBool::set(Priority priority, const std::string & value) {
    set(priority, from_string(value));
}

void OptionBool::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

std::string OptionBool::to_string(bool value) const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

OptionBool * OptionBool::clone() const {
    return new OptionBool(*this);
}

void OptionBool::test(bool /*unused*/) const {}

bool OptionBool::get_value() const noexcept {
    return p_impl->value;
}

bool OptionBool::get_default_value() const noexcept {
    return p_impl->default_value;
}

std::string OptionBool::get_value_string() const {
    return to_string(p_impl->value);
}

const std::vector<std::string> & OptionBool::get_default_true_values() noexcept {
    static std::vector<std::string> true_values = {"1", "yes", "true", "on"};
    return true_values;
}

const std::vector<std::string> & OptionBool::get_default_false_values() noexcept {
    static std::vector<std::string> false_values = {"0", "no", "false", "off"};
    return false_values;
}

const std::vector<std::string> & OptionBool::get_true_values() const noexcept {
    return p_impl->true_values ? *p_impl->true_values : get_default_true_values();
}

const std::vector<std::string> & OptionBool::get_false_values() const noexcept {
    return p_impl->false_values ? *p_impl->false_values : get_default_false_values();
}

}  // namespace libdnf5
