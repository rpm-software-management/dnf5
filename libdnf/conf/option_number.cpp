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

#include "libdnf5/conf/option_number.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"

#include <limits>
#include <sstream>

namespace libdnf5 {

template <typename T>
bool from_string(T & out, const std::string & in, std::ios_base & (*manipulator)(std::ios_base &)) {
    std::istringstream iss(in);
    return !(iss >> manipulator >> out).fail();
}

template <typename T>
OptionNumber<T>::OptionNumber(T default_value, T min, T max)
    : Option(Priority::DEFAULT),
      default_value(default_value),
      min(min),
      max(max),
      value(default_value) {
    test(default_value);
}

template <typename T>
OptionNumber<T>::OptionNumber(T default_value, T min)
    : OptionNumber(default_value, min, std::numeric_limits<T>::max()) {}

template <typename T>
OptionNumber<T>::OptionNumber(T default_value)
    : OptionNumber(default_value, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()) {}

template <typename T>
OptionNumber<T>::OptionNumber(T default_value, T min, T max, FromStringFunc && from_string_func)
    : Option(Priority::DEFAULT),
      from_string_user(std::move(from_string_func)),
      default_value(default_value),
      min(min),
      max(max),
      value(default_value) {
    test(default_value);
}

template <typename T>
OptionNumber<T>::OptionNumber(T default_value, T min, FromStringFunc && from_string_func)
    : OptionNumber(default_value, min, std::numeric_limits<T>::max(), std::move(from_string_func)) {}

template <typename T>
OptionNumber<T>::OptionNumber(T default_value, FromStringFunc && from_string_func)
    : OptionNumber(
          default_value, std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), std::move(from_string_func)) {}

template <typename T>
void OptionNumber<T>::test(ValueType value) const {
    if (value < min || value > max) {
        throw OptionValueNotAllowedError(M_("Input value {} is outside the allowed range {} ... {}"), value, min, max);
    }
}

template <typename T>
T OptionNumber<T>::from_string(const std::string & value) const {
    if (from_string_user) {
        return from_string_user(value);
    }
    ValueType val;
    if (libdnf::from_string<ValueType>(val, value, std::dec)) {
        return val;
    }
    throw OptionInvalidValueError(M_("Invalid number option value \"{}\""), value);
}

template <typename T>
void OptionNumber<T>::set(Priority priority, ValueType value) {
    assert_not_locked();

    if (priority >= get_priority()) {
        test(value);
        this->value = value;
        set_priority(priority);
    }
}

template <typename T>
void OptionNumber<T>::set(ValueType value) {
    set(Priority::RUNTIME, value);
}

template <typename T>
void OptionNumber<T>::set(Option::Priority priority, const std::string & value) {
    set(priority, from_string(value));
}

template <typename T>
void OptionNumber<T>::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

template <typename T>
std::string OptionNumber<T>::to_string(ValueType value) const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template class OptionNumber<std::int32_t>;
template class OptionNumber<std::uint32_t>;
template class OptionNumber<std::int64_t>;
template class OptionNumber<std::uint64_t>;
template class OptionNumber<float>;

}  // namespace libdnf5
