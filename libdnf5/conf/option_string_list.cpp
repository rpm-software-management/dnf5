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
    Impl(T && default_value) : icase(false), default_value(std::move(default_value)) {
        add_items(Priority::DEFAULT, this->default_value, false, "");
    }

    Impl(T && default_value, std::string && regex, bool icase)
        : regex(std::move(regex)),
          icase(icase),
          default_value(std::move(default_value)) {
        init_regex_matcher();
        test(this->default_value);
        add_items(Priority::DEFAULT, this->default_value, false, "");
    }

    Impl(T && default_value, std::string && regex, bool icase, std::string && delimiters)
        : regex(std::move(regex)),
          icase(icase),
          delimiters(std::move(delimiters)),
          default_value(std::move(default_value)) {
        init_regex_matcher();
        test(this->default_value);
        add_items(Priority::DEFAULT, this->default_value, false, "");
    }

    Impl(const std::string & default_value) : icase(false) {
        this->default_value = from_string(default_value);
        add_items(Priority::DEFAULT, this->default_value, false, "");
    }

    Impl(const std::string & default_value, std::string regex, bool icase) : regex(std::move(regex)), icase(icase) {
        this->default_value = from_string(default_value);
        init_regex_matcher();
        test(this->default_value);
        add_items(Priority::DEFAULT, this->default_value, false, "");
    }

    void test(const ValueType & value) const;

    void test_item(const std::string & item) const;

    T from_string(std::string value) const;

    /// Returns the default delimiters
    static const char * get_default_delimiters() noexcept;

    /// Return delimiters of this OptionStringList
    const char * get_delimiters() const noexcept;

    void add_items(Priority priority, const ValueType & items, bool empty_remove_existing, std::string source);

    std::vector<ItemInfo<std::string_view>> get_items_info() const;

private:
    friend OptionStringContainer;

    struct AppendEntry {
        ValueType items;
        bool remove_existing;
        std::string source;
    };

    void init_regex_matcher();
    void test_item_worker(const std::string & item) const;

    std::optional<std::regex> regex_matcher;
    std::string regex;
    bool icase;
    std::optional<std::string> delimiters;
    ValueType default_value;
    ValueType value;

    // For append options all user-set attempts are remembered in this multimap.
    // Each entry stores the value along with the source that set it
    // ("" = set without source, non-empty = set with named source).
    // The items are kept sorted by priority, and the final value is computed
    // these entries in priority order.
    std::multimap<Priority, AppendEntry> value_append;

    struct ItemOrigin {
        Priority priority;
        const std::string * source;
    };
    using InternalItemsInfo = std::conditional_t<
        std::is_same_v<T, std::vector<std::string>>,
        std::vector<ItemOrigin>,
        std::map<std::string_view, ItemOrigin>>;
    InternalItemsInfo internal_items_info;
};


template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::Impl::test_item_worker(const std::string & item) const {
    if (!std::regex_match(item, *regex_matcher)) {
        throw OptionValueNotAllowedError(
            M_("Input item value \"{}\" not allowed, allowed values for this option are defined by regular expression "
               "\"{}\""),
            item,
            regex);
    }
}


template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::Impl::test_item(const std::string & item) const {
    if (regex.empty()) {
        return;
    }
    test_item_worker(item);
}


template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::Impl::test(const ValueType & value) const {
    if (regex.empty()) {
        return;
    }
    for (const auto & val : value) {
        test_item_worker(val);
    }
}


template <typename T, bool IsAppend>
T OptionStringContainer<T, IsAppend>::Impl::from_string(std::string value) const {
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
inline const char * OptionStringContainer<T, IsAppend>::Impl::get_default_delimiters() noexcept {
    return ", \n";
}


template <typename T, bool IsAppend>
inline const char * OptionStringContainer<T, IsAppend>::Impl::get_delimiters() const noexcept {
    return delimiters ? delimiters->c_str() : get_default_delimiters();
}


template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::Impl::add_items(
    Priority priority, const ValueType & items, bool empty_remove_existing, std::string source) {
    if constexpr (IsAppend) {
        // if empty_remove_existing == true then empty value or first empty item clears
        // remove existing items from the result
        const bool remove_existing = empty_remove_existing && (items.empty() || items.begin()->empty());
        value_append.insert({priority, {items, remove_existing, std::move(source)}});

        // Recompute from value_append
        value.clear();
        internal_items_info.clear();
        for (const auto & [prio, entry] : value_append) {
            if (entry.remove_existing) {
                value.clear();
                internal_items_info.clear();
            }
            for (const auto & item : entry.items) {
                if (!entry.remove_existing || !item.empty()) {
                    value.insert(value.end(), item);
                    if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                        internal_items_info.emplace_back(prio, &entry.source);
                    } else {
                        internal_items_info.insert_or_assign(
                            internal_items_info.end(), item, ItemOrigin{prio, &entry.source});
                    }
                }
            }
        }
    } else {
        // In non-append container value_append is needed to store items source
        const auto it = value_append.insert({priority, {items, false, std::move(source)}});

        const std::string * const src = &it->second.source;
        for (const auto & item : it->second.items) {
            value.insert(value.end(), item);
            if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                internal_items_info.emplace_back(priority, src);
            } else {
                internal_items_info.insert_or_assign(internal_items_info.end(), item, ItemOrigin{priority, src});
            }
        }
    }
}


template <typename T, bool IsAppend>
std::vector<Option::ItemInfo<std::string_view>> OptionStringContainer<T, IsAppend>::Impl::get_items_info() const {
    std::vector<Option::ItemInfo<std::string_view>> items_info;
    items_info.reserve(value.size());
    if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        for (std::size_t idx = 0; idx < value.size(); ++idx) {
            items_info.emplace_back(internal_items_info[idx].priority, value[idx], *internal_items_info[idx].source);
        }
    } else {
        for (const auto & [val, origin] : internal_items_info) {
            items_info.emplace_back(origin.priority, val, *origin.source);
        }
    }
    return items_info;
}


template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::Impl::init_regex_matcher() {
    if (regex.empty()) {
        return;
    }

    auto flags = std::regex::ECMAScript | std::regex::nosubs;
    if (icase) {
        flags |= std::regex::icase;
    }
    regex_matcher = std::regex(regex, flags);
}


template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(ValueType default_value)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value))) {}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(ValueType default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value), std::move(regex), icase)) {}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(const std::string & default_value)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(default_value)) {}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(
    const std::string & default_value, std::string regex, bool icase)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(default_value, std::move(regex), icase)) {}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(
    ValueType default_value, std::string regex, bool icase, std::string delimiters)
    : Option(Priority::DEFAULT),
      p_impl(new Impl(std::move(default_value), std::move(regex), icase, std::move(delimiters))) {}

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::OptionStringContainer(const OptionStringContainer & src) = default;

template <typename T, bool IsAppend>
OptionStringContainer<T, IsAppend>::~OptionStringContainer() = default;

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::test_item(const std::string & item) const {
    p_impl->test_item(item);
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::test(const ValueType & value) const {
    p_impl->test(value);
}

template <typename T, bool IsAppend>
T OptionStringContainer<T, IsAppend>::from_string(std::string value) const {
    return p_impl->from_string(value);
}


template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(Priority priority, const ValueType & value) {
    set(priority, value, take_pending_source());
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(Priority priority, const ValueType & value, std::string source) {
    assert_not_locked();

    if constexpr (IsAppend) {
        test(value);
        p_impl->add_items(priority, value, true, source);
        if (priority >= get_priority()) {
            set_priority(priority);
            set_source(std::move(source));
        }
    } else {
        if (priority >= get_priority()) {
            test(value);
            p_impl->internal_items_info.clear();
            p_impl->value_append.clear();
            p_impl->value.clear();
            p_impl->add_items(priority, value, false, source);
            set_priority(priority);
            set_source(std::move(source));
        }
    }
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(const ValueType & value) {
    set(Priority::RUNTIME, value, take_pending_source());
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::set(const ValueType & value, std::string source) {
    set(Priority::RUNTIME, value, std::move(source));
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
    add(priority, items, take_pending_source());
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add(Priority priority, const ValueType & items, std::string source) {
    assert_not_locked();

    test(items);
    if constexpr (IsAppend) {
        p_impl->add_items(priority, items, false, source);
        if (priority >= get_priority()) {
            set_priority(priority);
            set_source(std::move(source));
        }
    } else {
        if (priority >= get_priority()) {
            p_impl->add_items(priority, items, false, source);
            set_priority(priority);
            set_source(std::move(source));
        }
    }
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add(Priority priority, const std::string & value) {
    add(priority, from_string(value));
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add(Priority priority, const std::string & value, std::string source) {
    add(priority, from_string(value), std::move(source));
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add_item(Priority priority, const std::string & item) {
    T items{item};
    add(priority, items);
}

template <typename T, bool IsAppend>
void OptionStringContainer<T, IsAppend>::add_item(Priority priority, const std::string & item, std::string source) {
    T items{item};
    add(priority, items, source);
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
    return Impl::get_default_delimiters();
}

template <typename T, bool IsAppend>
inline const char * OptionStringContainer<T, IsAppend>::get_delimiters() const noexcept {
    return p_impl->get_delimiters();
}


template <typename T, bool IsAppend>
std::vector<Option::ItemInfo<std::string_view>> OptionStringContainer<T, IsAppend>::get_items_info() const {
    return p_impl->get_items_info();
}


template class OptionStringContainer<std::vector<std::string>, true>;
template class OptionStringContainer<std::set<std::string>, true>;
template class OptionStringContainer<std::vector<std::string>, false>;
template class OptionStringContainer<std::set<std::string>, false>;

}  // namespace libdnf5
