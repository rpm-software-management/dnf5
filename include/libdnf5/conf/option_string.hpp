// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CONF_OPTION_STRING_HPP
#define LIBDNF5_CONF_OPTION_STRING_HPP

#include "option.hpp"


namespace libdnf5 {

/// Option that stores string value.
/// Support default value, and check of an input value using the regular expression
// @replaces libdnf:conf/OptionString.hpp:class:OptionString
class LIBDNF_API OptionString : public Option {
public:
    using ValueType = std::string;

    explicit OptionString(std::string default_value);
    explicit OptionString(const char * default_value);
    OptionString(std::string default_value, std::string regex, bool icase);
    OptionString(const char * default_value, std::string regex, bool icase);

    ~OptionString();

    OptionString(const OptionString & src);

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionString.hpp:method:OptionString.clone()
    OptionString * clone() const override;

    /// Sets new value and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionString.hpp:method:OptionString.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Gets the stored value.
    // @replaces libdnf:conf/OptionString.hpp:method:OptionString.getValue()
    const std::string & get_value() const;

    /// Gets the default value. Default value is used until it is replaced by set() method.
    // @replaces libdnf:conf/OptionString.hpp:method:OptionString.getValueString()
    const std::string & get_default_value() const noexcept;

    /// Gets a string representation of the stored value.
    // @replaces libdnf:conf/OptionString.hpp:method:OptionString.getValueString()
    std::string get_value_string() const override;

    /// Tests input value and throws exception if the value is not allowed.
    // @replaces libdnf:conf/OptionString.hpp:method:OptionString.test(const std::string & value)
    void test(const std::string & value) const;

    /// Returns copy of input string. Must be present for compatibility with other option types.
    // @replaces libdnf:conf/OptionString.hpp:method:OptionString.fromString(const std::string & value)
    std::string from_string(const std::string & value) const;

private:
    friend class OptionPath;
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
