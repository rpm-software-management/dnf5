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

#ifndef LIBDNF5_CONF_OPTION_NUMBER_HPP
#define LIBDNF5_CONF_OPTION_NUMBER_HPP

#include "option.hpp"

#include <functional>


namespace libdnf5 {

/// Option that stores numerical value. The type of value is template parameter.
/// Support default value, minimal and maximal values, user defined function for conversion from string.
// @replaces libdnf:conf/OptionNumber.hpp:class:OptionNumber<T>
template <typename T>
class LIBDNF_API OptionNumber : public Option {
public:
    using ValueType = T;
    using FromStringFunc = std::function<ValueType(const std::string &)>;

    /// Constructor that sets default value and limits for min and max values.
    // @replaces libdnf:conf/OptionNumber.hpp:ctor:OptionNumber.OptionNumber<T>(T default_value, T min, T max)
    OptionNumber(T default_value, T min, T max);

    /// Constructor that sets default value and limit for min value.
    // @replaces libdnf:conf/OptionNumber.hpp:ctor:OptionNumber.OptionNumber<T>(T default_value, T min)
    OptionNumber(T default_value, T min);

    /// Constructor that sets default value.
    // @replaces libdnf:conf/OptionNumber.hpp:ctor:OptionNumber.OptionNumber<T>(T default_value)
    explicit OptionNumber(T default_value);

    /// Constructor that sets default value and limits for min and max values and input parsing function.
    // @replaces libdnf:conf/OptionNumber.hpp:ctor:OptionNumber.OptionNumber<T>(T default_value, T min, T max, FromStringFunc && fromStringFunc)
    OptionNumber(T default_value, T min, T max, FromStringFunc && from_string_func);

    /// Constructor that sets default value and limit for min value and input parsing function.
    // @replaces libdnf:conf/OptionNumber.hpp:ctor:OptionNumber.OptionNumber<T>(T default_value, T min, FromStringFunc && fromStringFunc)
    OptionNumber(T default_value, T min, FromStringFunc && from_string_func);

    /// Constructor that sets default value and input parsing function.
    // @replaces libdnf:conf/OptionNumber.hpp:ctor:OptionNumber.OptionNumber<T>(T default_value, FromStringFunc && fromStringFunc)
    OptionNumber(T default_value, FromStringFunc && from_string_func);

    OptionNumber(const OptionNumber & src);

    ~OptionNumber() override;

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionBoo<T>.clone()
    OptionNumber * clone() const override;

    /// Sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.set(Priority priority, bool value)
    void set(Priority priority, ValueType value);

    /// Sets new value and runtime priority.
    void set(ValueType value);

    /// Parses input string and sets new value and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Parses input string and sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Gets the stored value.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.getValue()
    T get_value() const;

    /// Gets the default value. Default value is used until it is replaced by set() method.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.getValueString()
    T get_default_value() const;

    /// Gets a string representation of the stored value.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.getValueString()
    std::string get_value_string() const override;

    /// Tests input value and throws exception if the value is not allowed.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.test(ValueType value)
    void test(ValueType value) const;

    /// Parses input string and returns result.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.fromString(const std::string & value)
    T from_string(const std::string & value) const;

    /// Converts input value to the string.
    // @replaces libdnf:conf/OptionNumber.hpp:method:OptionNumber<T>.toString(ValueType value)
    std::string to_string(ValueType value) const;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

extern template class OptionNumber<std::int32_t>;
extern template class OptionNumber<std::uint32_t>;
extern template class OptionNumber<std::int64_t>;
extern template class OptionNumber<std::uint64_t>;
extern template class OptionNumber<float>;

}  // namespace libdnf5

#endif
