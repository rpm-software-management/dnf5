// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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

    ~OptionEnum() override;

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
