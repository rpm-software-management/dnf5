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

namespace libdnf5 {

class Option::Impl {
public:
    Impl(Priority priority) : priority(priority) {};

private:
    friend Option;

    Priority priority;
    bool locked{false};
    std::string lock_comment;
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

}  // namespace libdnf5
