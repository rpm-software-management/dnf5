/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LIBDNF_OPTION_CHILD_HPP
#define _LIBDNF_OPTION_CHILD_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Option.hpp"

namespace libdnf {

template <class ParentOptionType, class Enable = void>
class OptionChild : public Option {
public:
    OptionChild(const ParentOptionType & parent);
    OptionChild * clone() const override;
    Priority getPriority() const override;
    void set(Priority priority, const typename ParentOptionType::ValueType & value);
    void set(Priority priority, const std::string & value) override;
    const typename ParentOptionType::ValueType getValue() const;
    const typename ParentOptionType::ValueType getDefaultValue() const;
    std::string getValueString() const override;
    bool empty() const noexcept override;

private:
    const ParentOptionType * parent;
    typename ParentOptionType::ValueType value;
};

template <class ParentOptionType>
class OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type> : public Option {
public:
    OptionChild(const ParentOptionType & parent);
    OptionChild * clone() const override;
    Priority getPriority() const override;
    void set(Priority priority, const std::string & value) override;
    const std::string & getValue() const;
    const std::string & getDefaultValue() const;
    std::string getValueString() const override;
    bool empty() const noexcept override;

private:
    const ParentOptionType * parent;
    std::string value;
};

template <class ParentOptionType, class Enable>
inline OptionChild<ParentOptionType, Enable>::OptionChild(const ParentOptionType & parent)
: parent(&parent) {}

template <class ParentOptionType, class Enable>
inline OptionChild<ParentOptionType, Enable> * OptionChild<ParentOptionType, Enable>::clone() const
{
    return new OptionChild<ParentOptionType>(*this);
}

template <class ParentOptionType, class Enable>
inline Option::Priority OptionChild<ParentOptionType, Enable>::getPriority() const
{
    return priority != Priority::EMPTY ? priority : parent->getPriority();
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(Priority priority, const typename ParentOptionType::ValueType & value)
{
    if (priority >= this->priority) {
        parent->test(value);
        this->priority = priority;
        this->value = value;
    }
}

template <class ParentOptionType, class Enable>
inline void OptionChild<ParentOptionType, Enable>::set(Priority priority, const std::string & value)
{
    if (priority >= this->priority)
        set(priority, parent->fromString(value));
}

template <class ParentOptionType, class Enable>
inline const typename ParentOptionType::ValueType OptionChild<ParentOptionType, Enable>::getValue() const
{
    return priority != Priority::EMPTY ? value : parent->getValue();
}

template <class ParentOptionType, class Enable>
inline const typename ParentOptionType::ValueType OptionChild<ParentOptionType, Enable>::getDefaultValue() const
{
    return parent->getDefaultValue();
}

template <class ParentOptionType, class Enable>
inline std::string OptionChild<ParentOptionType, Enable>::getValueString() const
{
    return priority != Priority::EMPTY ? parent->toString(value) : parent->getValueString();
}

template <class ParentOptionType, class Enable>
inline bool OptionChild<ParentOptionType, Enable>::empty() const noexcept
{
    return priority == Priority::EMPTY && parent->empty();
}

template <class ParentOptionType>
inline OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::OptionChild(const ParentOptionType & parent)
: parent(&parent) {}

template <class ParentOptionType>
inline OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type> *
OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::clone() const
{
    return new OptionChild<ParentOptionType>(*this);
}

template <class ParentOptionType>
inline Option::Priority OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getPriority() const
{
    return priority != Priority::EMPTY ? priority : parent->getPriority();
}

template <class ParentOptionType>
inline void OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::set(Priority priority, const std::string & value)
{
    auto val = parent->fromString(value);
    if (priority >= this->priority) {
        parent->test(val);
        this->priority = priority;
        this->value = val;
    }
}

template <class ParentOptionType>
inline const std::string & OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getValue() const
{
    return priority != Priority::EMPTY ? value : parent->getValue();
}

template <class ParentOptionType>
inline const std::string & OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getDefaultValue() const
{
    return parent->getDefaultValue();
}

template <class ParentOptionType>
inline std::string OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::getValueString() const
{
    return priority != Priority::EMPTY ? value : parent->getValue();
}

template <class ParentOptionType>
inline bool OptionChild<ParentOptionType, typename std::enable_if<std::is_same<typename ParentOptionType::ValueType, std::string>::value>::type>::empty() const noexcept
{
    return priority == Priority::EMPTY && parent->empty();
}

}

#endif

#endif
