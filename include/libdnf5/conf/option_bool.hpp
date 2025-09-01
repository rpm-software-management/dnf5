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

#ifndef LIBDNF5_CONF_OPTION_BOOL_HPP
#define LIBDNF5_CONF_OPTION_BOOL_HPP

#include "option.hpp"

#include <vector>


namespace libdnf5 {

/// Option that stores boolean value.
/// Supports default value.
/// Conversion from string to bool is done according to vectors which contains strings of true and false values.
/// Conversion is case insensitive for input. Values must be lower case in vectors.
// @replaces libdnf:conf/OptionBool.hpp:class:OptionBool
class LIBDNF_API OptionBool : public Option {
public:
    using ValueType = bool;

    /// Constructor that sets default value.
    // @replaces libdnf:conf/OptionBool.hpp:ctor:OptionBool.OptionBool(bool default_value)
    explicit OptionBool(bool default_value);

    /// Constructor that sets default value and vectors for conversion from string.
    // @replaces libdnf:conf/OptionBool.hpp:ctor:OptionBool.OptionBool(bool defaultValue, const char * const trueVals[], const char * const falseVals[]);
    OptionBool(bool default_value, std::vector<std::string> true_vals, std::vector<std::string> false_vals);

    OptionBool(const OptionBool & src);

    ~OptionBool() override;

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
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5

#endif
