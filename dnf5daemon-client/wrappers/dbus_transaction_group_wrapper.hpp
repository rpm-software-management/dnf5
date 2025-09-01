// Copyright Contributors to the DNF5 project.
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

#ifndef DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_GROUP_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_GROUP_WRAPPER_HPP

#include "dbus_group_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <vector>


namespace dnfdaemon::client {

class DbusTransactionGroupWrapper {
public:
    explicit DbusTransactionGroupWrapper(const dnfdaemon::DbusTransactionItem & dti)
        : action(libdnf5::transaction::transaction_item_action_from_string(std::get<1>(dti))),
          reason(libdnf5::transaction::transaction_item_reason_from_string(std::get<2>(dti))),
          group(std::get<4>(dti)) {}

    DbusGroupWrapper get_group() const { return group; }
    libdnf5::transaction::TransactionItemAction get_action() const { return action; }
    libdnf5::transaction::TransactionItemReason get_reason() const { return reason; }

private:
    libdnf5::transaction::TransactionItemAction action;
    libdnf5::transaction::TransactionItemReason reason;
    DbusGroupWrapper group;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPER_DBUS_GROUP_WRAPPER_HPP
