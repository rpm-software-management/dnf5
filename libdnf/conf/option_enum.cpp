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

#include "libdnf/conf/option_enum.hpp"

#include <algorithm>
#include <sstream>

namespace libdnf {

template <typename T>
bool from_string(T & out, const std::string & in, std::ios_base & (*manipulator)(std::ios_base &)) {
    std::istringstream iss(in);
    return !(iss >> manipulator >> out).fail();
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType default_value, const std::vector<ValueType> & enum_vals)
    : Option(Priority::DEFAULT)
    , enum_vals(enum_vals)
    , default_value(default_value)
    , value(default_value) {
    test(default_value);
}

template <typename T>
OptionEnum<T>::OptionEnum(ValueType default_value, std::vector<ValueType> && enum_vals)
    : Option(Priority::DEFAULT)
    , enum_vals(std::move(enum_vals))
    , default_value(default_value)
    , value(default_value) {
    test(default_value);
}

template <typename T>
OptionEnum<T>::OptionEnum(
    ValueType default_value, const std::vector<ValueType> & enum_vals, FromStringFunc && from_string_func)
    : Option(Priority::DEFAULT)
    , from_string_user(std::move(from_string_func))
    , enum_vals(enum_vals)
    , default_value(default_value)
    , value(default_value) {
    test(default_value);
}

template <typename T>
OptionEnum<T>::OptionEnum(
    ValueType default_value, std::vector<ValueType> && enum_vals, FromStringFunc && from_string_func)
    : Option(Priority::DEFAULT)
    , from_string_user(std::move(from_string_func))
    , enum_vals(std::move(enum_vals))
    , default_value(default_value)
    , value(default_value) {
    test(default_value);
}

template <typename T>
void OptionEnum<T>::test(ValueType value) const {
    auto it = std::find(enum_vals.begin(), enum_vals.end(), value);
    if (it == enum_vals.end()) {
        throw NotAllowedValue(to_string(value));
    }
}

template <typename T>
T OptionEnum<T>::from_string(const std::string & value) const {
    if (from_string_user) {
        return from_string_user(value);
    }
    T val;
    if (libdnf::from_string<ValueType>(val, value, std::dec)) {
        return val;
    }
    throw InvalidValue(value);
}

template <typename T>
void OptionEnum<T>::set(Priority priority, ValueType value) {
    if (is_locked()) {
        throw WriteLocked(get_lock_comment());
    }
    if (priority >= this->priority) {
        test(value);
        this->value = value;
        this->priority = priority;
    }
}

template <typename T>
void OptionEnum<T>::set(Priority priority, const std::string & value) {
    set(priority, from_string(value));
}

template <typename T>
T OptionEnum<T>::get_value() const {
    return value;
}

template <typename T>
T OptionEnum<T>::get_default_value() const {
    return default_value;
}

template <typename T>
std::string OptionEnum<T>::to_string(ValueType value) const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template <typename T>
std::string OptionEnum<T>::get_value_string() const {
    return to_string(value);
}

OptionEnum<std::string>::OptionEnum(const std::string & default_value, std::vector<ValueType> enum_vals)
    : Option(Priority::DEFAULT)
    , enum_vals(std::move(enum_vals))
    , default_value(default_value)
    , value(default_value) {
    test(default_value);
}

OptionEnum<std::string>::OptionEnum(
    const std::string & default_value, std::vector<ValueType> enum_vals, FromStringFunc && from_string_func)
    : Option(Priority::DEFAULT)
    , from_string_user(std::move(from_string_func))
    , enum_vals(std::move(enum_vals))
    , default_value(default_value)
    , value(default_value) {
    test(default_value);
}

void OptionEnum<std::string>::test(const std::string & value) const {
    auto it = std::find(enum_vals.begin(), enum_vals.end(), value);
    if (it == enum_vals.end()) {
        throw NotAllowedValue(value);
    }
}

std::string OptionEnum<std::string>::from_string(const std::string & value) const {
    if (from_string_user) {
        return from_string_user(value);
    }
    return value;
}

void OptionEnum<std::string>::set(Priority priority, const std::string & value) {
    if (is_locked()) {
        throw WriteLocked(get_lock_comment());
    }
    auto val = from_string(value);
    if (priority >= get_priority()) {
        test(val);
        this->value = val;
        set_priority(priority);
    }
}

}  // namespace libdnf
