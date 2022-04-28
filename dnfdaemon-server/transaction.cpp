/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "transaction.hpp"

#include <libdnf/common/exception.hpp>
#include <libdnf/rpm/transaction_callbacks.hpp>

#include <type_traits>


namespace dnfdaemon {

RpmTransactionItemActions transaction_package_to_action(const libdnf::base::TransactionPackage & tspkg) {
    switch (tspkg.get_action()) {
        case libdnf::base::TransactionPackage::Action::INSTALL:
            return RpmTransactionItemActions::INSTALL;
        case libdnf::transaction::TransactionItemAction::UPGRADE:
            return RpmTransactionItemActions::UPGRADE;
        case libdnf::base::TransactionPackage::Action::DOWNGRADE:
            return RpmTransactionItemActions::DOWNGRADE;
        case libdnf::base::TransactionPackage::Action::REINSTALL:
            return RpmTransactionItemActions::REINSTALL;
        case libdnf::base::TransactionPackage::Action::REMOVE:
        case libdnf::transaction::TransactionItemAction::REPLACED:
            return RpmTransactionItemActions::ERASE;
        case libdnf::base::TransactionPackage::Action::REINSTALLED:
        case libdnf::base::TransactionPackage::Action::UPGRADED:
        case libdnf::base::TransactionPackage::Action::DOWNGRADED:
        case libdnf::base::TransactionPackage::Action::OBSOLETE:
        case libdnf::base::TransactionPackage::Action::REASON_CHANGE:
        case libdnf::transaction::TransactionItemAction::OBSOLETED:
            // TODO(lukash) handle cases
            libdnf_throw_assertion(
                "Unexpected action in RpmTransactionItem: {}",
                static_cast<std::underlying_type_t<libdnf::base::TransactionPackage::Action>>(tspkg.get_action()));
    }
    libdnf_throw_assertion(
        "Unknown action in RpmTransactionItem: {}",
        static_cast<std::underlying_type_t<libdnf::base::TransactionPackage::Action>>(tspkg.get_action()));
}

}  // namespace dnfdaemon
