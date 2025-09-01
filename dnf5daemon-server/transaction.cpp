// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "transaction.hpp"

#include <libdnf5/common/exception.hpp>
#include <libdnf5/rpm/transaction_callbacks.hpp>

#include <type_traits>


namespace dnfdaemon {

RpmTransactionItemActions transaction_package_to_action(const libdnf5::base::TransactionPackage & tspkg) {
    switch (tspkg.get_action()) {
        case libdnf5::base::TransactionPackage::Action::INSTALL:
            return RpmTransactionItemActions::INSTALL;
        case libdnf5::transaction::TransactionItemAction::UPGRADE:
            return RpmTransactionItemActions::UPGRADE;
        case libdnf5::base::TransactionPackage::Action::DOWNGRADE:
            return RpmTransactionItemActions::DOWNGRADE;
        case libdnf5::base::TransactionPackage::Action::REINSTALL:
            return RpmTransactionItemActions::REINSTALL;
        case libdnf5::base::TransactionPackage::Action::REMOVE:
        case libdnf5::transaction::TransactionItemAction::REPLACED:
            return RpmTransactionItemActions::ERASE;
        case libdnf5::base::TransactionPackage::Action::REASON_CHANGE:
        case libdnf5::base::TransactionPackage::Action::ENABLE:
        case libdnf5::base::TransactionPackage::Action::DISABLE:
        case libdnf5::base::TransactionPackage::Action::RESET:
        case libdnf5::base::TransactionPackage::Action::SWITCH:
            // TODO(lukash) handle cases
            libdnf_throw_assertion(
                "Unexpected action in RpmTransactionItem: {}",
                static_cast<std::underlying_type_t<libdnf5::base::TransactionPackage::Action>>(tspkg.get_action()));
    }
    libdnf_throw_assertion(
        "Unknown action in RpmTransactionItem: {}",
        static_cast<std::underlying_type_t<libdnf5::base::TransactionPackage::Action>>(tspkg.get_action()));
}

}  // namespace dnfdaemon
