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

#include "libdnf5/base/transaction_module.hpp"

#include "transaction_module_impl.hpp"

#include <utility>

namespace libdnf5::base {

TransactionModule::~TransactionModule() = default;

TransactionModule::TransactionModule(const TransactionModule & mpkg) : p_impl(new Impl(*mpkg.p_impl)) {}
TransactionModule::TransactionModule(TransactionModule && mpkg) noexcept = default;

TransactionModule & TransactionModule::operator=(const TransactionModule & mpkg) {
    if (this != &mpkg) {
        if (p_impl) {
            *p_impl = *mpkg.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*mpkg.p_impl);
        }
    }

    return *this;
}
TransactionModule & TransactionModule::operator=(TransactionModule && mpkg) noexcept = default;

TransactionModule::Impl::Impl(std::string module_name, std::string module_stream, Action action, Reason reason)
    : action(action),
      reason(reason),
      module_name(std::move(module_name)),
      module_stream(std::move(module_stream)) {}

TransactionModule::TransactionModule(
    const std::string & module_name, const std::string & module_stream, Action action, Reason reason)
    : p_impl(std::make_unique<Impl>(module_name, module_stream, action, reason)) {}

transaction::TransactionItemAction TransactionModule::get_action() const noexcept {
    return p_impl->action;
}

transaction::TransactionItemState TransactionModule::get_state() const noexcept {
    return p_impl->state;
}

transaction::TransactionItemReason TransactionModule::get_reason() const noexcept {
    return p_impl->reason;
}

std::string TransactionModule::get_module_name() const {
    return p_impl->module_name;
}

std::string TransactionModule::get_module_stream() const {
    return p_impl->module_stream;
}

std::vector<std::pair<std::string, std::string>> TransactionModule::get_replaces() const noexcept {
    return p_impl->replaces;
}

const std::vector<std::pair<std::string, std::string>> & TransactionModule::get_replaced_by() const noexcept {
    return p_impl->replaced_by;
}

void TransactionModule::Impl::replaces_append(std::string && module_name, std::string && module_stream) {
    replaces.push_back(std::make_pair(std::move(module_name), std::move(module_stream)));
}

void TransactionModule::Impl::replaced_by_append(std::string && module_name, std::string && module_stream) {
    replaced_by.push_back(std::make_pair(std::move(module_name), std::move(module_stream)));
}

}  // namespace libdnf5::base
