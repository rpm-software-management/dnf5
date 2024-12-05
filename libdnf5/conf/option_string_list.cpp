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

#include <map>
#include <optional>
#include <regex>

namespace libdnf5 {

template <typename T, bool IsAppend>
class OptionStringContainer<T, IsAppend>::Impl {
public:
    Impl(T && default_value)
        : icase(false),
          default_value(default_value),
          value(default_value),
          value_append({{Option::Priority::DEFAULT, std::move(default_value)}}) {};
    Impl(T && default_value, std::string && regex, bool icase)
        : regex(std::move(regex)),
          icase(icase),
          default_value(default_value),
          value(default_value),
          value_append({{Option::Priority::DEFAULT, std::move(default_value)}}) {};
    Impl(T && default_value, std::string && regex, bool icase, std::string && delimiters)
        : regex(std::move(regex)),
          icase(icase),
          delimiters(std::move(delimiters)),
          default_value(default_value),
          value(default_value),
          value_append({{Option::Priority::DEFAULT, std::move(default_value)}}) {};

    // For append options the method stores the value to value_append multimap and
    // then recalculates this->value.
    // For plain container it directly stores the value to this->value.
    void set_value(Priority priority, const ValueType & value);

private:
    friend OptionStringContainer;

    std::optional<std::regex> regex_matcher;
    std::string regex;
    bool icase;
    std::optional<std::string> delimiters;
    ValueType default_value;
    ValueType value;

    // For append options all set attempts are remembered in this multimap.
    // The items in the multimap are kept sorted according to the key (here,
    // Priority), ensuring correct behavior of resetting the value using an
    // empty item.
    std::multimap<Priority, ValueType> value_append;
};

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::Impl::set_value(Priority priority, const ValueType & value) {
    if (IsAppend) {
        value_append.insert({priority, value});

        // Compute the actual value of the append option by traversing all
        // change attempts in priority order (ensured by multimap) and either
        // adding the items to the end of the list or clearing the list.
        ValueType retval{};
        for (const auto & [priority, values] : value_append) {
            if (values.empty()) {
                // setting to empty value clears the result
                retval.clear();
                continue;
            }
            bool first = true;
            for (const auto & item : values) {
                if (item.empty()) {
                    // if the first item in the list is empty, clear the result
                    if (first) {
                        retval.clear();
                    }
                } else {
                    retval.insert(retval.end(), item);
                }
                first = false;
            }
        }
        this->value = retval;
    } else {
        this->value = value;
    }
}


template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(ValueType default_value)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value))) {}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(ValueType default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value), std::move(regex), icase)) {
    init_regex_matcher();
    test(default_value);
}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(const std::string & default_value)
    : Option(Priority::DEFAULT),
      p_impl(new Impl({})) {
    p_impl->default_value = from_string(default_value);
    p_impl->set_value(Priority::DEFAULT, p_impl->default_value);
}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(
    const std::string & default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      p_impl(new Impl({}, std::move(regex), icase)) {
    init_regex_matcher();
    p_impl->default_value = from_string(default_value);
    test(p_impl->default_value);
    p_impl->set_value(Priority::DEFAULT, p_impl->default_value);
}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(
    ValueType default_value, std::string regex, bool icase, std::string delimiters)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value), std::move(regex), icase, std::move(delimiters))) {
    init_regex_matcher();
    test(p_impl->default_value);
}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(const OptionStringContainer & src) = default;

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::~OptionStringContainer() = default;

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::init_regex_matcher() {
    if (p_impl->regex.empty()) {
        return;
    }

    auto flags = std::regex::ECMAScript | std::regex::nosubs;
    if (p_impl->icase) {
        flags |= std::regex::icase;
    }
    p_impl->regex_matcher = std::regex(p_impl->regex, flags);
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::test_item_worker(const std::string & item) const {
    if (!std::regex_match(item, *p_impl->regex_matcher)) {
        throw OptionValueNotAllowedError(
            M_("Input item value \"{}\" not allowed, allowed values for this option are defined by regular expression "
               "\"{}\""),
            item,
            p_impl->regex);
    }
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::test_item(const std::string & item) const {
    if (p_impl->regex.empty()) {
        return;
    }
    test_item_worker(item);
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::test(const ValueType & value) const {
    if (p_impl->regex.empty()) {
        return;
    }
    for (const auto & val : value) {
        test_item_worker(val);
    }
}

// Since strtok_r modifies its str input we copy `value` param
template <typename T, bool IsAppend>
T OptionStringContainer<T, IsAppend>::from_string(std::string value) const {
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

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(Priority priority, const ValueType & value) {
    assert_not_locked();

    if (IsAppend) {
        // for append options set is the same as add
        add(priority, value);
    } else {
        if (priority >= get_priority()) {
            test(value);
            p_impl->set_value(priority, value);
            set_priority(priority);
        }
    }
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(const ValueType & value) {
    set(Priority::RUNTIME, value);
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(Priority priority, const std::string & value) {
    set(priority, from_string(value));
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add(Priority priority, const ValueType & items) {
    assert_not_locked();

    test(items);
    if (IsAppend) {
        p_impl->set_value(priority, items);
        if (priority >= get_priority()) {
            set_priority(priority);
        }
    } else {
        if (priority >= get_priority()) {
            for (const auto & item : items) {
                p_impl->value.insert(p_impl->value.end(), item);
            }
            set_priority(priority);
        }
    }
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add(Priority priority, const std::string & value) {
    add(priority, from_string(value));
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add_item(Priority priority, const std::string & item) {
    assert_not_locked();

    test_item(item);
    p_impl->value.insert(p_impl->value.end(), item);
    if (get_priority() < priority) {
        set_priority(priority);
    }
}

template <typename T, bool IsAppend>
std::string OptionStringContainer<T, IsAppend>::to_string(const ValueType & value) const {
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

template <typename T, bool IsAppend>
inline OptionStringContainer<T, IsAppend> * OptionStringContainer<T, IsAppend>::clone() const {
    return new OptionStringContainer<T, IsAppend>(*this);
}

template <typename T, bool IsAppend>
inline const T & OptionStringContainer<T, IsAppend>::get_value() const {
    return p_impl->value;
}

template <typename T, bool IsAppend>
inline const T & OptionStringContainer<T, IsAppend>::get_default_value() const {
    return p_impl->default_value;
}

template <typename T, bool IsAppend>
inline std::string OptionStringContainer<T, IsAppend>::get_value_string() const {
    return to_string(p_impl->value);
}

template <typename T, bool IsAppend>
inline const char * OptionStringContainer<T, IsAppend>::get_default_delimiters() noexcept {
    return " ,\n";
}

template <typename T, bool IsAppend>
inline const char * OptionStringContainer<T, IsAppend>::get_delimiters() const noexcept {
    return p_impl->delimiters ? p_impl->delimiters->c_str() : get_default_delimiters();
}


template class OptionStringContainer<std::vector<std::string>, true>;
template class OptionStringContainer<std::set<std::string>, true>;
template class OptionStringContainer<std::vector<std::string>, false>;
template class OptionStringContainer<std::set<std::string>, false>;

}  // namespace libdnf5
