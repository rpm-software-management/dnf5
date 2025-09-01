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

#include "libdnf5/base/transaction_package.hpp"

#include "transaction_package_impl.hpp"

#include <utility>

namespace libdnf5::base {

TransactionPackage::~TransactionPackage() = default;

TransactionPackage::TransactionPackage(const TransactionPackage & mpkg) : p_impl(new Impl(*mpkg.p_impl)) {}
TransactionPackage::TransactionPackage(TransactionPackage && mpkg) noexcept = default;

TransactionPackage & TransactionPackage::operator=(const TransactionPackage & mpkg) {
    if (this != &mpkg) {
        if (p_impl) {
            *p_impl = *mpkg.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*mpkg.p_impl);
        }
    }

    return *this;
}
TransactionPackage & TransactionPackage::operator=(TransactionPackage && mpkg) noexcept = default;

TransactionPackage::Impl::Impl(
    libdnf5::rpm::Package pkg, Action action, Reason reason, State state, std::optional<std::string> group_id)
    : package(std::move(pkg)),
      state(state),
      action(action),
      reason(reason),
      reason_change_group_id(std::move(group_id)) {}

TransactionPackage::TransactionPackage(const libdnf5::rpm::Package & pkg, Action action, Reason reason)
    : p_impl(std::make_unique<Impl>(pkg, action, reason, State::STARTED)) {}

TransactionPackage::TransactionPackage(const libdnf5::rpm::Package & pkg, Action action, Reason reason, State state)
    : p_impl(std::make_unique<Impl>(pkg, action, reason, state)) {}

TransactionPackage::TransactionPackage(
    const libdnf5::rpm::Package & pkg, Action action, Reason reason, const std::optional<std::string> & group_id)
    : p_impl(std::make_unique<Impl>(pkg, action, reason, State::STARTED, group_id)) {}


transaction::TransactionItemAction TransactionPackage::get_action() const noexcept {
    return p_impl->action;
}

transaction::TransactionItemState TransactionPackage::get_state() const noexcept {
    return p_impl->state;
}

transaction::TransactionItemReason TransactionPackage::get_reason() const noexcept {
    return p_impl->reason;
}

libdnf5::rpm::Package TransactionPackage::get_package() const {
    return p_impl->package;
}

const std::string * TransactionPackage::get_reason_change_group_id() const noexcept {
    return p_impl->reason_change_group_id ? &p_impl->reason_change_group_id.value() : nullptr;
}

std::vector<rpm::Package> TransactionPackage::get_replaces() const noexcept {
    return p_impl->replaces;
}

const std::vector<rpm::Package> & TransactionPackage::get_replaced_by() const noexcept {
    return p_impl->replaced_by;
}

void TransactionPackage::Impl::replaced_by_append(rpm::Package && pkg) {
    replaced_by.push_back(std::move(pkg));
}

void TransactionPackage::Impl::replaces_append(rpm::Package && pkg) {
    replaces.push_back(std::move(pkg));
}

void TransactionPackage::Impl::set_reason(Reason value) noexcept {
    reason = value;
}

}  // namespace libdnf5::base
