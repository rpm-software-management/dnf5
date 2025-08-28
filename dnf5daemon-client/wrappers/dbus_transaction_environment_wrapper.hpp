// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_ENVIRONMENT_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_ENVIRONMENT_WRAPPER_HPP

#include "dbus_environment_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <vector>


namespace dnfdaemon::client {

class DbusTransactionEnvironmentWrapper {
public:
    explicit DbusTransactionEnvironmentWrapper(const dnfdaemon::DbusTransactionItem & dti)
        : action(libdnf5::transaction::transaction_item_action_from_string(std::get<1>(dti))),
          reason(libdnf5::transaction::transaction_item_reason_from_string(std::get<2>(dti))),
          environment(std::get<4>(dti)) {}

    DbusEnvironmentWrapper get_environment() const { return environment; }
    libdnf5::transaction::TransactionItemAction get_action() const { return action; }
    libdnf5::transaction::TransactionItemReason get_reason() const { return reason; }

private:
    libdnf5::transaction::TransactionItemAction action;
    libdnf5::transaction::TransactionItemReason reason;
    DbusEnvironmentWrapper environment;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPER_DBUS_ENVIRONMENT_WRAPPER_HPP
