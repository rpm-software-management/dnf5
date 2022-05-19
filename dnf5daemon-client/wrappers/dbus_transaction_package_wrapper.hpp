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

#ifndef DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_PACKAGE_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_PACKAGE_WRAPPER_HPP

#include "dbus_package_wrapper.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf/transaction/transaction_item_action.hpp>
#include <libdnf/transaction/transaction_item_reason.hpp>

#include <vector>


namespace dnfdaemon::client {

class DbusTransactionPackageWrapper {
public:
    explicit DbusTransactionPackageWrapper(const dnfdaemon::DbusTransactionItem & dti)
        : package(std::get<1>(dti)),
          action(static_cast<libdnf::transaction::TransactionItemAction>(std::get<0>(dti))),
          // TODO(lukash) reason needs to be added to dbus
          reason(libdnf::transaction::TransactionItemReason::NONE) {}

    DbusPackageWrapper get_package() const { return package; }
    libdnf::transaction::TransactionItemAction get_action() const { return action; }
    libdnf::transaction::TransactionItemReason get_reason() const { return reason; }
    // TODO(jmracek) get_replaces() is only a dummy method. In future it requires a private setter and a way how to get
    // data from dnf-deamon server
    const std::vector<DbusPackageWrapper> get_replaces() const noexcept { return {}; }

private:
    DbusPackageWrapper package;
    libdnf::transaction::TransactionItemAction action;
    libdnf::transaction::TransactionItemReason reason;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP
