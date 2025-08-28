// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_PACKAGE_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_PACKAGE_WRAPPER_HPP

#include "dbus_package_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <vector>


namespace dnfdaemon::client {

class DbusTransactionPackageWrapper {
public:
    explicit DbusTransactionPackageWrapper(const dnfdaemon::DbusTransactionItem & dti)
        : action(libdnf5::transaction::transaction_item_action_from_string(std::get<1>(dti))),
          reason(libdnf5::transaction::transaction_item_reason_from_string(std::get<2>(dti))),
          transaction_item_attrs(std::get<3>(dti)),
          package(std::get<4>(dti)) {}

    const DbusPackageWrapper & get_package() const noexcept { return package; }
    DbusPackageWrapper & get_package() noexcept { return package; }
    libdnf5::transaction::TransactionItemAction get_action() const noexcept { return action; }
    libdnf5::transaction::TransactionItemReason get_reason() const noexcept { return reason; }
    dnfdaemon::KeyValueMap & get_transaction_item_attrs() noexcept { return transaction_item_attrs; }
    // TODO(jmracek) get_replaces() is only a dummy method. In future it requires a private setter and a way how to get
    // data from dnf-deamon server
    const std::vector<DbusPackageWrapper> & get_replaces() const noexcept { return replaces; }
    void set_replaces(std::vector<DbusPackageWrapper> && replaces) { this->replaces = replaces; }

private:
    libdnf5::transaction::TransactionItemAction action;
    libdnf5::transaction::TransactionItemReason reason;
    dnfdaemon::KeyValueMap transaction_item_attrs;
    DbusPackageWrapper package;
    std::vector<DbusPackageWrapper> replaces;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP
