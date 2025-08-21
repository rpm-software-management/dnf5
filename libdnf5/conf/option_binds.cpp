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

#include "libdnf5/conf/option_binds.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <utility>

namespace libdnf5 {

OptionBindsOptionNotFoundError::OptionBindsOptionNotFoundError(const std::string & id)
    : OptionBindsError(M_("Option \"{}\" not found"), id) {}

OptionBindsOptionAlreadyExistsError::OptionBindsOptionAlreadyExistsError(const std::string & id)
    : OptionBindsError(M_("Option \"{}\" already exists"), id) {}

// ========== OptionBinds::Item class ===============
class OptionBinds::Item::Impl {
public:
    Impl(Option & option, NewStringFunc && new_string_func, GetValueStringFunc && get_value_string_func, bool add_value)
        : option(&option),
          new_str_func(std::move(new_string_func)),
          get_value_str_func(std::move(get_value_string_func)),
          is_append_option(add_value) {}

    Impl(Option & option) : option(&option) {};

private:
    friend OptionBinds::Item;
    Option * option;
    NewStringFunc new_str_func;
    GetValueStringFunc get_value_str_func;
    bool is_append_option{false};  // hint that new value be added/appended
};

OptionBinds::Item::Item(
    Option & option, NewStringFunc new_string_func, GetValueStringFunc get_value_string_func, bool add_value)
    : p_impl(new Impl(option, std::move(new_string_func), std::move(get_value_string_func), add_value)) {}

OptionBinds::Item::Item(Option & option) : p_impl(new Impl(option)) {}

OptionBinds::Item::~Item() = default;

OptionBinds::Item::Item() = default;
OptionBinds::Item::Item(const Item & src) = default;
OptionBinds::Item & OptionBinds::Item::operator=(const Item & src) = default;

Option::Priority OptionBinds::Item::get_priority() const {
    return p_impl->option->get_priority();
}

void OptionBinds::Item::new_string(Option::Priority priority, const std::string & value) {
    if (p_impl->new_str_func) {
        p_impl->new_str_func(priority, value);
    } else {
        p_impl->option->set(priority, value);
    }
}

std::string OptionBinds::Item::get_value_string() const {
    if (p_impl->get_value_str_func) {
        return p_impl->get_value_str_func();
    }
    return p_impl->option->get_value_string();
}

bool OptionBinds::Item::get_is_append_option() const {
    return p_impl->is_append_option;
}


// =========== OptionBinds class ===============
class OptionBinds::Impl {
private:
    friend OptionBinds;

    Container items;
};

OptionBinds::OptionBinds() : p_impl(new Impl()) {}
OptionBinds::OptionBinds(const OptionBinds & src) = default;
OptionBinds::~OptionBinds() = default;

OptionBinds::Item & OptionBinds::at(const std::string & id) {
    auto item = p_impl->items.find(id);
    if (item == p_impl->items.end()) {
        throw OptionBindsOptionNotFoundError(id);
    }
    return item->second;
}

const OptionBinds::Item & OptionBinds::at(const std::string & id) const {
    auto item = p_impl->items.find(id);
    if (item == p_impl->items.end()) {
        throw OptionBindsOptionNotFoundError(id);
    }
    return item->second;
}

OptionBinds::Item & OptionBinds::add(
    std::string id,
    Option & option,
    Item::NewStringFunc new_string_func,
    Item::GetValueStringFunc get_value_string_func,
    bool add_value) {
    auto item = p_impl->items.find(id);
    if (item != p_impl->items.end()) {
        throw OptionBindsOptionAlreadyExistsError(id);
    }
    auto res = p_impl->items.emplace(
        std::move(id), Item(option, std::move(new_string_func), std::move(get_value_string_func), add_value));
    return res.first->second;
}

OptionBinds::Item & OptionBinds::add(std::string id, Option & option) {
    auto item = p_impl->items.find(id);
    if (item != p_impl->items.end()) {
        throw OptionBindsOptionAlreadyExistsError(id);
    }
    auto res = p_impl->items.emplace(std::move(id), Item(option));
    return res.first->second;
}

bool OptionBinds::empty() const noexcept {
    return p_impl->items.empty();
}
std::size_t OptionBinds::size() const noexcept {
    return p_impl->items.size();
}
OptionBinds::iterator OptionBinds::begin() noexcept {
    return p_impl->items.begin();
}
OptionBinds::const_iterator OptionBinds::begin() const noexcept {
    return p_impl->items.begin();
}
OptionBinds::const_iterator OptionBinds::cbegin() const noexcept {
    return p_impl->items.cbegin();
}
OptionBinds::iterator OptionBinds::end() noexcept {
    return p_impl->items.end();
}
OptionBinds::const_iterator OptionBinds::end() const noexcept {
    return p_impl->items.end();
}
OptionBinds::const_iterator OptionBinds::cend() const noexcept {
    return p_impl->items.cend();
}
OptionBinds::iterator OptionBinds::find(const std::string & id) {
    return p_impl->items.find(id);
}
OptionBinds::const_iterator OptionBinds::find(const std::string & id) const {
    return p_impl->items.find(id);
}

}  // namespace libdnf5
