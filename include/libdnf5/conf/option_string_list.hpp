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

#ifndef LIBDNF5_CONF_OPTION_STRING_LIST_HPP
#define LIBDNF5_CONF_OPTION_STRING_LIST_HPP

#include "option.hpp"

#include <set>
#include <vector>


namespace libdnf5 {

/// Option that stores a container of strings. The type of the container is a
/// template parameter. Non-type IsAppend template parameter is used to
/// distinguish between regular list-like options (e.g. OptionStringList,
/// OptionStringSet) and append options (e.g. OptionStringAppendList,
/// OptionStringAppendSet).
/// Support default value, and check of an input value using the regular expression.
// @replaces libdnf:conf/OptionStringList.hpp:class:OptionStringList
template <typename T, bool IsAppend = false>
class OptionStringContainer : public Option {
public:
    using ValueType = T;

    explicit OptionStringContainer(ValueType default_value);
    OptionStringContainer(ValueType default_value, std::string regex, bool icase);
    explicit OptionStringContainer(const std::string & default_value);
    OptionStringContainer(const std::string & default_value, std::string regex, bool icase);
    OptionStringContainer(ValueType default_value, std::string regex, bool icase, std::string delimiters);

    OptionStringContainer(const OptionStringContainer & src);
    ~OptionStringContainer() override;

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.clone()
    OptionStringContainer * clone() const override;

    /// Sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionStingList.hpp:method:OptionStringList.set(Priority priority, bool value)
    virtual void set(Priority priority, const ValueType & value);

    /// Sets new value and runtime priority.
    void set(const ValueType & value);

    /// Parses input string and sets new value and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Parses input string and sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Adds items from an another container.
    /// New items are stored in the container value
    void add(Priority priority, const ValueType & items);

    /// Parses input string and adds new values and priority.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    void add(Priority priority, const std::string & value);

    /// Adds new item to the container.
    /// New item is stored in the container value
    void add_item(Priority priority, const std::string & item);

    /// Gets the stored value.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.getValue()
    const ValueType & get_value() const;

    /// Gets the default value. Default value is used until it is replaced by set() method.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.getValueString()
    const ValueType & get_default_value() const;

    /// Gets a string representation of the stored value.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.getValueString()
    std::string get_value_string() const override;

    /// Tests input value and throws exception if the value is not allowed.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.test(const std::vector<std::string> & value)
    void test(const ValueType & value) const;

    /// Parses input string and returns result.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.fromString(const std::string & value)
    ValueType from_string(std::string value) const;

    /// Converts input value to the string.
    // @replaces libdnf:conf/OptionStringList.hpp:method:OptionStringList.toString(const ValueType & value)
    std::string to_string(const ValueType & value) const;

    /// Returns the default delimiters
    static const char * get_default_delimiters() noexcept;

    /// Return delimiters of this OptionStringList
    const char * get_delimiters() const noexcept;

protected:
    /// Tests new container item value and throws exception if the item value is not allowed.
    void test_item(const std::string & item) const;

private:
    void init_regex_matcher();
    void test_item_worker(const std::string & item) const;

    class Impl;
    ImplPtr<Impl> p_impl;
};

using OptionStringList = OptionStringContainer<std::vector<std::string>>;
using OptionStringSet = OptionStringContainer<std::set<std::string>>;

using OptionStringAppendList = OptionStringContainer<std::vector<std::string>, true>;
using OptionStringAppendSet = OptionStringContainer<std::set<std::string>, true>;

}  // namespace libdnf5

#endif
