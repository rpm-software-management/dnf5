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

#include "libdnf5/conf/option_string_list.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <cctype>
#include <map>
#include <optional>
#include <regex>
#include <string_view>

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

template <typename T, bool IsAppend>
T OptionStringContainer<T, IsAppend>::from_string(std::string value) const {
    ValueType ret;  // Container to hold the resulting parsed items

    const std::string_view delimiters{get_delimiters()};

    bool item_started = false;  // True after first non-space or escaped character in current item
    size_t valid_item_len = 0;  // Length of current item excluding trailing unescaped spaces
    bool escape = false;        // True when backslash was encountered (next char is escaped)
    std::string item;           // Current item being built character by character

    const auto end_it = value.end();
    for (auto it = value.begin(); it != end_it; ++it) {
        // Handle escaped character: always add it
        if (escape) {
            escape = false;
            item += *it;
            valid_item_len = item.size();
            item_started = true;
            continue;
        }

        if (*it == '\\') {
            escape = true;
            continue;
        }

        // Character is not a delimiter - process as part of current item
        if (delimiters.find(*it) == delimiters.npos) {
            // Non-space character starts the item or extends its valid length
            if (!std::isspace(*it)) {
                item_started = true;
                valid_item_len = item.size() + 1;
            }
            // Append character to item only after item has started (skip leading spaces)
            if (item_started) {
                item += *it;
            }
            continue;
        }

        // Delimiter encountered - finalize and store current item
        item.resize(valid_item_len);  // Trim trailing unescaped spaces

        // Store item in result container with special handling for empty items:
        // - Empty first item with non-space delimiter: stored to signal content clearing
        // - Other empty items: skipped (consecutive delimiters produce no item)
        if (!item.empty() || (ret.empty() && !std::isspace(*it))) {
            ret.insert(ret.end(), item);
        }

        // Prepare state for parsing next item
        item_started = false;
        valid_item_len = 0;
        item.clear();
    }

    // Store the final item if it contains any content
    item.resize(valid_item_len);  // Trim trailing unescaped spaces
    if (!item.empty()) {
        ret.insert(ret.end(), item);
    }

    return ret;
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
    std::string result;

    const std::string_view delimiters{get_delimiters()};

    bool next{false};
    for (auto & item : value) {
        if (next) {
            if (!delimiters.empty()) {
                result += delimiters[0];
            }
        } else {
            next = true;
        }
        for (const auto ch : item) {
            if (ch == '\\' || delimiters.find(ch) != delimiters.npos) {
                result += '\\';
            }
            result += ch;
        }
    }

    return result;
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
    return ", \n";
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
