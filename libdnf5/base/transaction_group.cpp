// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/transaction_group.hpp"

#include <utility>

namespace libdnf5::base {

class TransactionGroup::Impl {
public:
    Impl(libdnf5::comps::Group grp, Action action, Reason reason, const PackageType & types);

private:
    friend TransactionGroup;

    State state{State::STARTED};
    Action action;
    Reason reason;
    libdnf5::comps::Group group;
    PackageType package_types;
};

TransactionGroup::~TransactionGroup() = default;

TransactionGroup::TransactionGroup(const TransactionGroup & mpkg) : p_impl(new Impl(*mpkg.p_impl)) {}
TransactionGroup::TransactionGroup(TransactionGroup && mpkg) noexcept = default;

TransactionGroup & TransactionGroup::operator=(const TransactionGroup & mpkg) {
    if (this != &mpkg) {
        if (p_impl) {
            *p_impl = *mpkg.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*mpkg.p_impl);
        }
    }

    return *this;
}
TransactionGroup & TransactionGroup::operator=(TransactionGroup && mpkg) noexcept = default;

TransactionGroup::Impl::Impl(libdnf5::comps::Group grp, Action action, Reason reason, const PackageType & types)
    : action(action),
      reason(reason),
      group(std::move(grp)),
      package_types(types) {}

TransactionGroup::TransactionGroup(
    const libdnf5::comps::Group & grp, Action action, Reason reason, const PackageType & types)
    : p_impl(std::make_unique<Impl>(grp, action, reason, types)) {}

libdnf5::comps::Group TransactionGroup::get_group() const {
    return p_impl->group;
}

transaction::TransactionItemAction TransactionGroup::get_action() const noexcept {
    return p_impl->action;
}

transaction::TransactionItemState TransactionGroup::get_state() const noexcept {
    return p_impl->state;
}

transaction::TransactionItemReason TransactionGroup::get_reason() const noexcept {
    return p_impl->reason;
}

libdnf5::comps::PackageType TransactionGroup::get_package_types() const noexcept {
    return p_impl->package_types;
}

}  // namespace libdnf5::base
