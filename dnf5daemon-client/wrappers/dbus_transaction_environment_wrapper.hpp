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

#ifndef DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_ENVIRONMENT_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_ENVIRONMENT_WRAPPER_HPP

#include "dbus_environment_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf/transaction/transaction_item_action.hpp>
#include <libdnf/transaction/transaction_item_reason.hpp>

#include <vector>


namespace dnfdaemon::client {

class DbusTransactionEnvironmentWrapper {
public:
    explicit DbusTransactionEnvironmentWrapper(const dnfdaemon::DbusTransactionItem & dti)
        : action(libdnf::transaction::transaction_item_action_from_string(std::get<1>(dti))),
          reason(libdnf::transaction::transaction_item_reason_from_string(std::get<2>(dti))),
          environment(std::get<4>(dti)) {}

    DbusEnvironmentWrapper get_environment() const { return environment; }
    libdnf::transaction::TransactionItemAction get_action() const { return action; }
    libdnf::transaction::TransactionItemReason get_reason() const { return reason; }

private:
    libdnf::transaction::TransactionItemAction action;
    libdnf::transaction::TransactionItemReason reason;
    DbusEnvironmentWrapper environment;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPER_DBUS_ENVIRONMENT_WRAPPER_HPP
