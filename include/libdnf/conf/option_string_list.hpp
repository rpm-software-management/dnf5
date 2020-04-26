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

#ifndef LIBDNF_CONF_OPTION_STRING_LIST_HPP
#define LIBDNF_CONF_OPTION_STRING_LIST_HPP

#include "option.hpp"

#include <vector>

namespace libdnf {

/// Option that stores list of strings.
/// Support default value, and check of an input value using the regular expression
/// @replaces libdnf:conf/OptionStringList.hpp:class:OptionStringList
class OptionStringList : public Option {
public:
    using ValueType = std::vector<std::string>;

    /// Exception that is generated when not allowed input value is detected.
    class NotAllowedValue : public InvalidValue {
    public:
        using InvalidValue::InvalidValue;
        const char * get_domain_name() const noexcept override { return "libdnf::OptionStringList"; }
        const char * get_name() const noexcept override { return "NotAllowedValue"; }
        const char * get_description() const noexcept override { return "Not allowed value"; }
    };

    explicit OptionStringList(const ValueType & default_value);
    OptionStringList(const ValueType & default_value, std::string regex, bool icase);
    explicit OptionStringList(const std::string & default_value);
    OptionStringList(const std::string & default_value, std::string regex, bool icase);

    /// Makes copy (clone) of this object.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.clone()
    OptionStringList * clone() const override;

    /// Sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    /// @replaces libdnf:conf/OptionStingList.hpp:method:OptionStringList.set(Priority priority, bool value)
    virtual void set(Priority priority, const ValueType & value);

    /// Parses input string and sets new value and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Gets the stored value.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.getValue()
    const ValueType & get_value() const;

    /// Gets the default value. Default value is used until it is replaced by set() method.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.getValueString()
    const ValueType & get_default_value() const;

    /// Gets a string representation of the stored value.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.getValueString()
    std::string get_value_string() const override;

    /// Tests input value and throws exception if the value is not allowed.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.test(const std::vector<std::string> & value)
    void test(const std::vector<std::string> & value) const;

    /// Parses input string and returns result.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.fromString(const std::string & value)
    ValueType from_string(const std::string & value) const;

    /// Converts input value to the string.
    /// @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.toString(const ValueType & value)
    std::string to_string(const ValueType & value) const;

protected:
    std::string regex;
    bool icase;
    ValueType default_value;
    ValueType value;
};

inline OptionStringList * OptionStringList::clone() const {
    return new OptionStringList(*this);
}

inline const OptionStringList::ValueType & OptionStringList::get_value() const {
    return value;
}

inline const OptionStringList::ValueType & OptionStringList::get_default_value() const {
    return default_value;
}

inline std::string OptionStringList::get_value_string() const {
    return to_string(value);
}

}  // namespace libdnf

#endif
