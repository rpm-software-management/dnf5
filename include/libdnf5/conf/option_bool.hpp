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

#ifndef LIBDNF5_CONF_OPTION_BOOL_HPP
#define LIBDNF5_CONF_OPTION_BOOL_HPP

#include "option.hpp"

#include <array>
#include <memory>
#include <vector>


namespace libdnf5 {

/// Option that stores boolean value.
/// Supports default value.
/// Conversion from string to bool is done according to vectors which contains strings of true and false values.
/// Conversion is case insensitive for input. Values must be lower case in vectors.
// @replaces libdnf:conf/OptionBool.hpp:class:OptionBool
class OptionBool : public Option {
public:
    using ValueType = bool;

    /// Constructor that sets default value.
    // @replaces libdnf:conf/OptionBool.hpp:ctor:OptionBool.OptionBool(bool default_value)
    explicit OptionBool(bool default_value);

    /// Constructor that sets default value and vectors for conversion from string.
    // @replaces libdnf:conf/OptionBool.hpp:ctor:OptionBool.OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[]);
    OptionBool(
        bool default_value, const std::vector<std::string> & true_vals, const std::vector<std::string> & false_vals);

    OptionBool(const OptionBool & src);

    ~OptionBool() override = default;

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.clone()
    OptionBool * clone() const override;

    /// Sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.set(Priority priority, bool value)
    void set(Priority priority, bool value);

    /// Sets new value with the runtime priority.
    void set(bool value);

    /// Parses input string and sets new value and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Parses input string and sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Gets the stored value.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.getValue()
    bool get_value() const noexcept;

    /// Gets the default value. Default value is used until it is replaced by set() method.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.getValueString()
    bool get_default_value() const noexcept;

    /// Gets a string representation of the stored value.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.getValueString()
    std::string get_value_string() const override;

    /// Returns vector with default strings used for conversion from string to "true" bool value.
    // @replaces libdnf:conf/OptionBool.hpp:global_const:defTrueValues[]
    static const std::vector<std::string> & get_default_true_values() noexcept;

    /// Returns vector with default strings used for conversion from string to "false" bool value.
    // @replaces libdnf:conf/OptionBool.hpp:global_const:defFalseValues[]
    static const std::vector<std::string> & get_default_false_values() noexcept;

    /// Returns vector with strings used for conversion from string to "true" bool value.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.getTrueValues()
    const std::vector<std::string> & get_true_values() const noexcept;

    /// Returns vector with strings used for conversion from string to "false" bool value.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.getFalseValues()
    const std::vector<std::string> & get_false_values() const noexcept;

    /// Does nothing. But it must be present for compatibility with other option types.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.test(bool /*unused*/)
    void test(bool /*unused*/) const;

    /// Parses input string and returns result.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.fromString(const std::string & value)
    bool from_string(const std::string & value) const;

    /// Converts input value to the string.
    // @replaces libdnf:conf/OptionBool.hpp:method:OptionBool.toString(bool value)
    std::string to_string(bool value) const;

private:
    std::unique_ptr<std::vector<std::string>> true_values;
    std::unique_ptr<std::vector<std::string>> false_values;
    bool default_value;
    bool value;
};


inline OptionBool * OptionBool::clone() const {
    return new OptionBool(*this);
}

inline void OptionBool::test(bool /*unused*/) const {}

inline bool OptionBool::get_value() const noexcept {
    return value;
}

inline bool OptionBool::get_default_value() const noexcept {
    return default_value;
}

inline std::string OptionBool::get_value_string() const {
    return to_string(value);
}

inline const std::vector<std::string> & OptionBool::get_default_true_values() noexcept {
    static std::vector<std::string> true_values = {"1", "yes", "true", "on"};
    return true_values;
}

inline const std::vector<std::string> & OptionBool::get_default_false_values() noexcept {
    static std::vector<std::string> false_values = {"0", "no", "false", "off"};
    return false_values;
}

inline const std::vector<std::string> & OptionBool::get_true_values() const noexcept {
    return true_values ? *true_values : get_default_true_values();
}

inline const std::vector<std::string> & OptionBool::get_false_values() const noexcept {
    return false_values ? *false_values : get_default_false_values();
}

}  // namespace libdnf5

#endif
