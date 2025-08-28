// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/transaction_environment.hpp"

#include <utility>

namespace libdnf5::base {

class TransactionEnvironment::Impl {
public:
    Impl(libdnf5::comps::Environment grp, Action action, Reason reason, bool with_optional);

private:
    friend TransactionEnvironment;

    State state{State::STARTED};
    Action action;
    bool with_optional;
    Reason reason;
    libdnf5::comps::Environment environment;
};

TransactionEnvironment::~TransactionEnvironment() = default;

TransactionEnvironment::TransactionEnvironment(const TransactionEnvironment & mpkg) : p_impl(new Impl(*mpkg.p_impl)) {}
TransactionEnvironment::TransactionEnvironment(TransactionEnvironment && mpkg) noexcept = default;

TransactionEnvironment & TransactionEnvironment::operator=(const TransactionEnvironment & mpkg) {
    if (this != &mpkg) {
        if (p_impl) {
            *p_impl = *mpkg.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*mpkg.p_impl);
        }
    }

    return *this;
}
TransactionEnvironment & TransactionEnvironment::operator=(TransactionEnvironment && mpkg) noexcept = default;

TransactionEnvironment::Impl::Impl(libdnf5::comps::Environment grp, Action action, Reason reason, bool with_optional)
    : action(action),
      with_optional(with_optional),
      reason(reason),
      environment(std::move(grp)) {}

TransactionEnvironment::TransactionEnvironment(
    const libdnf5::comps::Environment & grp, Action action, Reason reason, bool with_optional)
    : p_impl(std::make_unique<Impl>(grp, action, reason, with_optional)) {}

libdnf5::comps::Environment TransactionEnvironment::get_environment() const {
    return p_impl->environment;
}

transaction::TransactionItemAction TransactionEnvironment::get_action() const noexcept {
    return p_impl->action;
}

transaction::TransactionItemState TransactionEnvironment::get_state() const noexcept {
    return p_impl->state;
}

transaction::TransactionItemReason TransactionEnvironment::get_reason() const noexcept {
    return p_impl->reason;
}

bool TransactionEnvironment::get_with_optional() const noexcept {
    return p_impl->with_optional;
}

}  // namespace libdnf5::base
