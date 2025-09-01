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

#ifndef LIBDNF5_BASE_TRANSACTION_PACKAGE_IMPL_HPP
#define LIBDNF5_BASE_TRANSACTION_PACKAGE_IMPL_HPP

#include "libdnf5/base/transaction_package.hpp"

namespace libdnf5::base {

class TransactionPackage::Impl {
public:
    Impl(
        libdnf5::rpm::Package pkg,
        Action action,
        Reason reason,
        State state,
        std::optional<std::string> group_id = std::nullopt);

    void replaced_by_append(rpm::Package && pkg);
    void replaces_append(rpm::Package && pkg);
    void set_reason(Reason value) noexcept;

private:
    friend TransactionPackage;

    libdnf5::rpm::Package package;
    State state;
    Action action;
    Reason reason;
    std::optional<std::string> reason_change_group_id;
    std::vector<rpm::Package> replaces;
    std::vector<rpm::Package> replaced_by;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_PACKAGE_IMPL_HPP
