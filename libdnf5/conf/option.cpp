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

#include "libdnf5/conf/option.hpp"

#include "utils/on_scope_exit.hpp"

#include <vector>

namespace libdnf5 {

class Option::Impl {
public:
    Impl(Priority priority) : priority(priority) {};

private:
    friend Option;

    Priority priority;
    bool locked{false};
    std::string lock_comment;
    std::string source;
    std::string pending_source;

    // Used by OptionChild to delegate get_source() to parent when no own value is set
    const Option * parent{nullptr};
};

Option::Option(Priority priority) : p_impl(new Impl(priority)) {}

Option::~Option() = default;

Option::Option(const Option & src) = default;

Option::Priority Option::get_priority() const {
    return p_impl->priority;
}

bool Option::empty() const noexcept {
    return p_impl->priority == Priority::EMPTY;
}

void Option::set_priority(Priority priority) {
    p_impl->priority = priority;
}

void Option::lock(const std::string & first_comment) {
    if (!p_impl->locked) {
        p_impl->lock_comment = first_comment;
        p_impl->locked = true;
    }
}

bool Option::is_locked() const noexcept {
    return p_impl->locked;
}

void Option::assert_not_locked() const {
    libdnf_user_assert(!p_impl->locked, "Attempting to write to a locked option: {}", get_lock_comment());
}

const std::string & Option::get_lock_comment() const noexcept {
    return p_impl->lock_comment;
}

void Option::set(Priority priority, const std::string & value, std::string source) {
    set_pending_source(std::move(source));
    libdnf5::utils::OnScopeExit clear{[this]() noexcept { clear_pending_source(); }};
    set(priority, value);
}

void Option::set(const std::string & value, std::string source) {
    set_pending_source(std::move(source));
    libdnf5::utils::OnScopeExit clear{[this]() noexcept { clear_pending_source(); }};
    set(value);
}

void Option::set_source(std::string source) {
    p_impl->source = std::move(source);
}

void Option::set_pending_source(std::string source) {
    p_impl->pending_source = std::move(source);
}

void Option::clear_pending_source() noexcept {
    p_impl->pending_source.clear();
}

std::string Option::take_pending_source() noexcept {
    std::string src = std::move(p_impl->pending_source);
    p_impl->pending_source.clear();
    return src;
}

// When this option has no own value (priority == EMPTY) and has a parent,
// delegate to the parent's source. This allows OptionChild to inherit
// the source from its parent option without needing a virtual get_source().
const std::string & Option::get_source() const noexcept {
    return (p_impl->priority == Priority::EMPTY && p_impl->parent) ? p_impl->parent->p_impl->source : p_impl->source;
}

void Option::set_parent(const Option * parent) noexcept {
    p_impl->parent = parent;
}

const Option * Option::get_parent() const noexcept {
    return p_impl->parent;
}

}  // namespace libdnf5
