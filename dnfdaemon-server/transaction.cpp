/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-server is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "transaction.hpp"

#include <fmt/format.h>


namespace dnfdaemon {

RpmTransactionItem::RpmTransactionItem(const libdnf::base::TransactionPackage & tspkg)
    : TransactionItem(tspkg.get_package()) {
    switch (tspkg.get_action()) {
        case libdnf::transaction::TransactionItemAction::INSTALL:
            action = Actions::INSTALL;
            break;
        case libdnf::transaction::TransactionItemAction::UPGRADE:
            action = Actions::UPGRADE;
            break;
        case libdnf::transaction::TransactionItemAction::DOWNGRADE:
            action = Actions::DOWNGRADE;
            break;
        case libdnf::transaction::TransactionItemAction::REINSTALL:
            action = Actions::REINSTALL;
            break;
        case libdnf::transaction::TransactionItemAction::REMOVE:
        case libdnf::transaction::TransactionItemAction::OBSOLETED:
            action = Actions::ERASE;
            break;
        case libdnf::transaction::TransactionItemAction::REINSTALLED:
        case libdnf::transaction::TransactionItemAction::UPGRADED:
        case libdnf::transaction::TransactionItemAction::DOWNGRADED:
        case libdnf::transaction::TransactionItemAction::OBSOLETE:
        case libdnf::transaction::TransactionItemAction::REASON_CHANGE:
            // TODO(lukash) handle cases
            throw libdnf::LogicError(fmt::format("Unexpected action in RpmTransactionItem: {}", tspkg.get_action()));
    }
}

}  // namespace dnfdaemon
