// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
