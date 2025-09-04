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

#ifndef LIBDNF5_CONF_OPTION_ENUM_HPP
#define LIBDNF5_CONF_OPTION_ENUM_HPP

#include "option.hpp"

#include <functional>
#include <vector>


namespace libdnf5 {

/// Option that stores value from enumeration of strings.
/// It supports default value.
/// It supports user defined function for conversion from string.
// @replaces libdnf:conf/OptionEnum.hpp:class:OptionEnum<std::string>
class LIBDNF_API OptionEnum : public Option {
public:
    using ValueType = std::string;
    using FromStringFunc = std::function<std::string(const std::string &)>;

    OptionEnum(std::string default_value, std::vector<std::string> enum_vals);
    OptionEnum(std::string default_value, std::vector<std::string> enum_vals, FromStringFunc && from_string_func);

    OptionEnum(const OptionEnum & src);

    ~OptionEnum() override;

    OptionEnum & operator=(const OptionEnum & other);

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionEnum.hpp:method:OptionEnum<std::string>.clone()
    OptionEnum * clone() const override;

    /// Parses input string and sets new value and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionEnum.hpp:method:OptionEnum<std::string>.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Parses input string and sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Gets the stored value.
    // @replaces libdnf:conf/OptionEnum.hpp:method:OptionEnum<std::string>.getValue()
    const std::string & get_value() const;

    /// Gets the default value. Default value is used until it is replaced by set() method.
    // @replaces libdnf:conf/OptionEnum.hpp:method:OptionEnum<std::string>.getValueString()
    const std::string & get_default_value() const;

    /// Gets a string representation of the stored value.
    // @replaces libdnf:conf/OptionEnum.hpp:method:OptionEnum<std::string>.getValueString()
    std::string get_value_string() const override;

    /// Tests input value and throws exception if the value is not allowed.
    // @replaces libdnf:conf/OptionEnum.hpp:method:OptionEnum<std::string>.test(const std::string & value)
    void test(const std::string & value) const;

    /// Parses input string and returns result.
    // @replaces libdnf:conf/OptionEnum.hpp:method:OptionEnum<std::string>.fromString(const std::string & value)
    std::string from_string(const std::string & value) const;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5

#endif
