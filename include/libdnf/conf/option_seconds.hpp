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

#ifndef LIBDNF_CONF_OPTION_SECONDS_HPP
#define LIBDNF_CONF_OPTION_SECONDS_HPP

#include "option_number.hpp"

namespace libdnf {

/// Option that stores an integer value of seconds.
/// Support default value, minimal and maximal values.
/// @replaces libdnf:conf/OptionSeconds.hpp:class:OptionSeconds
class OptionSeconds : public OptionNumber<std::int32_t> {
public:
    /// Exception that is generated when an invalid input value is detected.
    class InvalidValue : public Option::InvalidValue {
    public:
        using Option::InvalidValue::InvalidValue;
        const char * get_domain_name() const noexcept override { return "libdnf::OptionSeconds"; }
    };

    /// Exception that is generated when an negative input value is detected.
    class NegativeValue : public InvalidValue {
    public:
        using InvalidValue::InvalidValue;
        const char * get_name() const noexcept override { return "NegativeValue"; }
        const char * get_description() const noexcept override { return "Seconds value must not be negative"; };
    };

    /// Exception that is generated when an unknown input value is detected.
    class UnknownUnit : public InvalidValue {
    public:
        using InvalidValue::InvalidValue;
        const char * get_name() const noexcept override { return "UnknownUnit"; }
        const char * get_description() const noexcept override { return "Unknown unit"; };
    };

    OptionSeconds(ValueType default_value, ValueType min, ValueType max);
    OptionSeconds(ValueType default_value, ValueType min);
    explicit OptionSeconds(ValueType default_value);

    /// Makes copy (clone) of this object.
    /// @replaces libdnf:conf/OptionSeconds.hpp:method:OptionSeconds.clone()
    OptionSeconds * clone() const override;

    using OptionNumber<std::int32_t>::set;

    /// Parses input string and sets new value and priority.
    /// Valid inputs: 100, 1.5m, 90s, 1.2d, 1d, 0xF, 0.1, -1, never.
    /// Invalid inputs: -10, -0.1, 45.6Z, 1d6h, 1day, 1y.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    /// @replaces libdnf:conf/OptionSeconds.hpp:method:OptionSeconds.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Parses input string and returns result.
    /// @replaces libdnf:conf/OptionSeconds.hpp:method:OptionSeconds.fromString(const std::string & value)
    ValueType from_string(const std::string & value) const;
};

inline OptionSeconds * OptionSeconds::clone() const {
    return new OptionSeconds(*this);
}

}  // namespace libdnf

#endif
