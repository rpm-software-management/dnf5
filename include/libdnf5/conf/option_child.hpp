// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_CONF_OPTION_CHILD_HPP
#define LIBDNF5_CONF_OPTION_CHILD_HPP

#include "option.hpp"


namespace libdnf5 {

/// Option that links option to another option. It uses default value and parameters from linked option.
/// If it is empty (has no stored value), uses value from the linked option (parent).
/// Parent option type is template parameter.
// @replaces libdnf:conf/OptionChild.hpp:class:OptionChild<T>
template <class ParentOptionType, class Enable = void>
class OptionChild : public Option {
public:
    /// Constructor takes reference to parent option.
    // @replaces libdnf:conf/OptionChild.hpp:ctor:OptionChild<T>.OptionChild(const ParentOptionType & parent)
    explicit OptionChild(const ParentOptionType & parent);

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.clone()
    OptionChild * clone() const override;

    /// Returns priority (source) of the stored value. If no value is stored, priority from the parent is returned.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.getPriority()
    Priority get_priority() const override;

    /// Sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.set(Priority priority, bool value)
    void set(Priority priority, const typename ParentOptionType::ValueType & value);

    /// Sets new value and runtime priority.
    void set(const typename ParentOptionType::ValueType & value);

    /// Sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.set(Priority priority, std::string value)
    void set(Priority priority, const std::string & value) override;

    /// Sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Gets the stored value. If no value is stored, value from the parent is returned.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.getValue()
    typename ParentOptionType::ValueType get_value() const;

    /// Gets the default value from parent. Default value is used until it is replaced by set() method.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.getValueString()
    typename ParentOptionType::ValueType get_default_value() const;

    /// Gets a string representation of the stored value. If no value is stored, value from the parent is returned.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.getValueString()
    std::string get_value_string() const override;

    /// Checks if the option is empty (has no stored value). If it is empty, checks status of the parent.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<T>.empty()
    bool empty() const noexcept override;

private:
    const ParentOptionType * parent;
    typename ParentOptionType::ValueType value;
};

/// Option that links option to another option. It uses default value and parameters from linked option.
/// If it is empty (has no stored value), uses value from the linked option (parent).
/// Parent option type is template parameter.
/// This is specialization for parent with std::string ValueType.
// @replaces libdnf:conf/OptionChild.hpp:class:OptionChild<std::string>
template <class ParentOptionType>
class OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>
    : public Option {
public:
    /// Constructor takes reference to parent option.
    // @replaces libdnf:conf/OptionChild.hpp:ctor:OptionChild.OptionChild<std::string>(const ParentOptionType & parent)
    explicit OptionChild(const ParentOptionType & parent);

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<std::string>.clone()
    OptionChild * clone() const override;

    /// Returns priority (source) of the stored value. If no value is stored, priority from the parent is returned.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<std::string>.getPriority()
    Priority get_priority() const override;

    /// Sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<std::string>.set(Priority priority, std::string value)
    void set(Priority priority, const std::string & value) override;

    /// Sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Gets the stored value. If no value is stored, value from the parent is returned.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<std::string>.getValue()
    const std::string & get_value() const;

    /// Gets the default value from parent. Default value is used until it is replaced by set() method.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<std::string>.getValueString()
    const std::string & get_default_value() const;

    /// Gets a string representation of the stored value. If no value is stored, value from the parent is returned.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<std::string>.getValueString()
    std::string get_value_string() const override;

    /// Checks if the option is empty (has no stored value). If it is empty, checks status of the parent.
    // @replaces libdnf:conf/OptionChild.hpp:method:OptionChild<std::string>.empty()
    bool empty() const noexcept override;

private:
    const ParentOptionType * parent;
    std::string value;
};

template <class ParentOptionType, class Enable>
inline OptionChild<ParentOptionType, Enable>::OptionChild(const ParentOptionType & parent) : parent(&parent) {}

template <class ParentOptionType, class Enable>
inline OptionChild<ParentOptionType, Enable> * OptionChild<ParentOptionType, Enable>::clone() const {
    return new OptionChild<ParentOptionType>(*this);
}

template <class ParentOptionType, class Enable>
inline Option::Priority OptionChild<ParentOptionType, Enable>::get_priority() const {
    return Option::get_priority() != Priority::EMPTY ? Option::get_priority() : parent->get_priority();
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(
    Priority priority, const typename ParentOptionType::ValueType & value) {
    assert_not_locked();

    if (priority >= Option::get_priority()) {
        parent->test(value);
        set_priority(priority);
        this->value = value;
    }
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(const typename ParentOptionType::ValueType & value) {
    set(Priority::RUNTIME, value);
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(Priority priority, const std::string & value) {
    set(priority, parent->from_string(value));
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

template <class ParentOptionType, class Enable>
inline typename ParentOptionType::ValueType OptionChild<ParentOptionType, Enable>::get_value() const {
    return Option::get_priority() != Priority::EMPTY ? value : parent->get_value();
}

template <class ParentOptionType, class Enable>
inline typename ParentOptionType::ValueType OptionChild<ParentOptionType, Enable>::get_default_value() const {
    return parent->get_default_value();
}

template <class ParentOptionType, class Enable>
inline std::string OptionChild<ParentOptionType, Enable>::get_value_string() const {
    return Option::get_priority() != Priority::EMPTY ? parent->to_string(value) : parent->get_value_string();
}

template <class ParentOptionType, class Enable>
inline bool OptionChild<ParentOptionType, Enable>::empty() const noexcept {
    return Option::get_priority() == Priority::EMPTY && parent->empty();
}

template <class ParentOptionType>
inline OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::
    OptionChild(const ParentOptionType & parent)
    : parent(&parent) {}

template <class ParentOptionType>
inline OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type> *
OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::clone()
    const {
    return new OptionChild<ParentOptionType>(*this);
}

template <class ParentOptionType>
inline Option::Priority OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::
    get_priority() const {
    return Option::get_priority() != Priority::EMPTY ? Option::get_priority() : parent->get_priority();
}

template <class ParentOptionType>
inline void OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::
    set(Priority priority, const std::string & value) {
    assert_not_locked();

    auto val = parent->from_string(value);
    if (priority >= Option::get_priority()) {
        parent->test(val);
        set_priority(priority);
        this->value = val;
    }
}

template <class ParentOptionType>
inline void OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::
    set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

template <class ParentOptionType>
inline const std::string & OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::get_value()
    const {
    return Option::get_priority() != Priority::EMPTY ? value : parent->get_value();
}

template <class ParentOptionType>
inline const std::string & OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::
    get_default_value() const {
    return parent->get_default_value();
}

template <class ParentOptionType>
inline std::string OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::
    get_value_string() const {
    return Option::get_priority() != Priority::EMPTY ? value : parent->get_value();
}

template <class ParentOptionType>
inline bool OptionChild<
    ParentOptionType,
    typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::empty()
    const noexcept {
    return Option::get_priority() == Priority::EMPTY && parent->empty();
}

}  // namespace libdnf5

#endif
