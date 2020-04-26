/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_CONF_OPTION_STRING_HPP
#define LIBDNF_CONF_OPTION_STRING_HPP

#include "option.hpp"

namespace libdnf {

/// Option that stores string value.
/// Support default value, and check of an input value using the regular expression
/// @replaces libdnf:conf/OptionString.hpp:class:OptionString
class OptionString : public Option {
public:
    using ValueType = std::string;

    /// Exception that is generated when an invalid input value is detected.
    class InvalidValue : public Option::InvalidValue {
    public:
        using Option::InvalidValue::InvalidValue;
        const char * get_domain_name() const noexcept override { return "libdnf::OptionString"; }
    };

    /// Exception that is generated when not allowed input value is detected.
    class NotAllowedValue : public InvalidValue {
    public:
        using InvalidValue::InvalidValue;
        const char * get_name() const noexcept override { return "NotAllowedValue"; }
        const char * get_description() const noexcept override { return "Not allowed value"; }
    };

    /// Exception that is generated during read an empty Option.
    class ValueNotSet : public Exception {
    public:
        ValueNotSet() : Exception("") {}
        const char * get_domain_name() const noexcept override { return "libdnf::OptionString"; }
        const char * get_name() const noexcept override { return "ValueNotSet"; }
        const char * get_description() const noexcept override { return "Value is not set"; }
    };

    explicit OptionString(const std::string & default_value);
    explicit OptionString(const char * default_value);
    OptionString(const std::string & default_value, std::string regex, bool icase);
    OptionString(const char * default_value, std::string regex, bool icase);

    /// Makes copy (clone) of this object.
    /// @replaces libdnf:conf/OptionString.hpp:method:OptionString.clone()
    OptionString * clone() const override;

    /// Sets new value and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    /// @replaces libdnf:conf/OptionString.hpp:method:OptionString.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Gets the stored value.
    /// @replaces libdnf:conf/OptionString.hpp:method:OptionString.getValue()
    const std::string & get_value() const;

    /// Gets the default value. Default value is used until it is replaced by set() method.
    /// @replaces libdnf:conf/OptionString.hpp:method:OptionString.getValueString()
    const std::string & get_default_value() const noexcept;

    /// Gets a string representation of the stored value.
    /// @replaces libdnf:conf/OptionString.hpp:method:OptionString.getValueString()
    std::string get_value_string() const override;

    /// Tests input value and throws exception if the value is not allowed.
    /// @replaces libdnf:conf/OptionString.hpp:method:OptionString.test(const std::string & value)
    void test(const std::string & value) const;

    /// Returns copy of input string. Must be present for compatibility with other option types.
    /// @replaces libdnf:conf/OptionString.hpp:method:OptionString.fromString(const std::string & value)
    std::string from_string(const std::string & value) const;

protected:
    std::string regex;
    bool icase;
    std::string default_value;
    std::string value;
};

inline OptionString * OptionString::clone() const {
    return new OptionString(*this);
}

inline const std::string & OptionString::get_default_value() const noexcept {
    return default_value;
}

inline std::string OptionString::get_value_string() const {
    return get_value();
}

inline std::string OptionString::from_string(const std::string & value) const {
    return value;
}

}  // namespace libdnf

#endif
