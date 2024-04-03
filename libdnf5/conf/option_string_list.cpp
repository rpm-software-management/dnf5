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

#include "libdnf5/conf/option_string_list.hpp"

#include "utils/string.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <optional>
#include <regex>

namespace libdnf5 {

template <typename T>
class OptionStringContainer<T>::Impl {
public:
    Impl(const T & default_value) : icase(false), default_value(default_value), value(default_value){};
    Impl(const T & default_value, std::string && regex, bool icase)
        : regex(std::move(regex)),
          icase(icase),
          default_value(default_value),
          value(default_value){};
    Impl(const T & default_value, std::string && regex, bool icase, std::string && delimiters)
        : regex(std::move(regex)),
          icase(icase),
          delimiters(std::move(delimiters)),
          default_value(default_value),
          value(default_value){};

private:
    friend OptionStringContainer;

    std::optional<std::regex> regex_matcher;
    std::string regex;
    bool icase;
    std::optional<std::string> delimiters;
    ValueType default_value;
    ValueType value;
};

template <typename T>
OptionStringContainer<T>::OptionStringContainer(const ValueType & default_value)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(default_value)) {}

template <typename T>
OptionStringContainer<T>::OptionStringContainer(const ValueType & default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(default_value, std::move(regex), icase)) {
    init_regex_matcher();
    test(default_value);
}

template <typename T>
OptionStringContainer<T>::OptionStringContainer(const std::string & default_value)
    : Option(Priority::DEFAULT),
      p_impl(new Impl({})) {
    p_impl->value = p_impl->default_value = from_string(default_value);
}

template <typename T>
OptionStringContainer<T>::OptionStringContainer(const std::string & default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      p_impl(new Impl({}, std::move(regex), icase)) {
    init_regex_matcher();
    p_impl->default_value = from_string(default_value);
    test(p_impl->default_value);
    p_impl->value = p_impl->default_value;
}

template <typename T>
OptionStringContainer<T>::OptionStringContainer(
    const ValueType & default_value, std::string regex, bool icase, std::string delimiters)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(default_value, std::move(regex), icase, std::move(delimiters))) {
    init_regex_matcher();
    test(p_impl->default_value);
}

template <typename T>
OptionStringContainer<T>::OptionStringContainer(const OptionStringContainer & src) = default;

template <typename T>
OptionStringContainer<T>::~OptionStringContainer() = default;

template <typename T>
void OptionStringContainer<T>::init_regex_matcher() {
    if (p_impl->regex.empty()) {
        return;
    }

    auto flags = std::regex::ECMAScript | std::regex::nosubs;
    if (p_impl->icase) {
        flags |= std::regex::icase;
    }
    p_impl->regex_matcher = std::regex(p_impl->regex, flags);
}

template <typename T>
void OptionStringContainer<T>::test_item_worker(const std::string & item) const {
    if (!std::regex_match(item, *p_impl->regex_matcher)) {
        throw OptionValueNotAllowedError(
            M_("Input item value \"{}\" not allowed, allowed values for this option are defined by regular expression "
               "\"{}\""),
            item,
            p_impl->regex);
    }
}

template <typename T>
void OptionStringContainer<T>::test_item(const std::string & item) const {
    if (p_impl->regex.empty()) {
        return;
    }
    test_item_worker(item);
}

template <typename T>
void OptionStringContainer<T>::test(const ValueType & value) const {
    if (p_impl->regex.empty()) {
        return;
    }
    for (const auto & val : value) {
        test_item_worker(val);
    }
}

// Since strtok_r modifies its str input we copy `value` param
template <typename T>
T OptionStringContainer<T>::from_string(std::string value) const {
    ValueType tmp;
    char * str = nullptr;
    char * token = nullptr;
    char * saveptr = nullptr;

    // If the first token in the string is empty (we start with a delimiter) it's
    // a special case, it can be used to clear existing content of the option.
    auto start = value.find_first_not_of(' ');
    if (start != std::string::npos && strchr(get_delimiters(), value[start]) != nullptr) {
        tmp.insert(tmp.end(), "");
    }

    for (str = value.data();; str = nullptr) {
        token = strtok_r(str, get_delimiters(), &saveptr);
        if (token == nullptr) {
            break;
        }
        std::string token_str(token);
        // Since deliemeters don't have to include space, we have to trim the token
        utils::string::trim(token_str);
        if (!token_str.empty()) {
            tmp.insert(tmp.end(), std::move(token_str));
        }
    }

    return tmp;
}

template <typename T>
void OptionStringContainer<T>::set(Priority priority, const ValueType & value) {
    assert_not_locked();

    if (priority >= get_priority()) {
        test(value);
        p_impl->value = value;
        set_priority(priority);
    }
}

template <typename T>
void OptionStringContainer<T>::set(const ValueType & value) {
    set(Priority::RUNTIME, value);
}

template <typename T>
void OptionStringContainer<T>::set(Priority priority, const std::string & value) {
    set(priority, from_string(value));
}

template <typename T>
void OptionStringContainer<T>::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

template <typename T>
void OptionStringContainer<T>::add(Priority priority, const ValueType & items) {
    assert_not_locked();

    test(items);
    for (const auto & item : items) {
        p_impl->value.insert(p_impl->value.end(), item);
    }
    if (get_priority() < priority) {
        set_priority(priority);
    }
}

template <typename T>
void OptionStringContainer<T>::add(Priority priority, const std::string & value) {
    add(priority, from_string(value));
}

template <typename T>
void OptionStringContainer<T>::add_item(Priority priority, const std::string & item) {
    assert_not_locked();

    test_item(item);
    p_impl->value.insert(p_impl->value.end(), item);
    if (get_priority() < priority) {
        set_priority(priority);
    }
}

template <typename T>
std::string OptionStringContainer<T>::to_string(const ValueType & value) const {
    std::ostringstream oss;
    bool next{false};
    for (auto & val : value) {
        if (next) {
            oss << ", ";
        } else {
            next = true;
        }
        oss << val;
    }
    return oss.str();
}

template <typename T>
inline OptionStringContainer<T> * OptionStringContainer<T>::clone() const {
    return new OptionStringContainer<T>(*this);
}

template <typename T>
inline const T & OptionStringContainer<T>::get_value() const {
    return p_impl->value;
}

template <typename T>
inline const T & OptionStringContainer<T>::get_default_value() const {
    return p_impl->default_value;
}

template <typename T>
inline std::string OptionStringContainer<T>::get_value_string() const {
    return to_string(p_impl->value);
}

template <typename T>
inline const char * OptionStringContainer<T>::get_default_delimiters() noexcept {
    return " ,\n";
}

template <typename T>
inline const char * OptionStringContainer<T>::get_delimiters() const noexcept {
    return p_impl->delimiters ? p_impl->delimiters->c_str() : get_default_delimiters();
}


template class OptionStringContainer<std::vector<std::string>>;
template class OptionStringContainer<std::set<std::string>>;

}  // namespace libdnf5
