// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_MODULE_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_MODULE_WRAPPER_HPP

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <vector>


namespace dnfdaemon::client {

class DbusTransactionModuleWrapper {
public:
    explicit DbusTransactionModuleWrapper(const dnfdaemon::DbusTransactionItem & dti)
        : action(libdnf5::transaction::transaction_item_action_from_string(std::get<1>(dti))),
          reason(libdnf5::transaction::transaction_item_reason_from_string(std::get<2>(dti))),
          // TODO(pkratoch): Get the actual name and stream
          module_name("<missing name>"),
          module_stream("<missing stream>") {}

    std::string get_module_name() const { return module_name; }
    std::string get_module_stream() const { return module_stream; }
    libdnf5::transaction::TransactionItemAction get_action() const { return action; }
    libdnf5::transaction::TransactionItemReason get_reason() const { return reason; }
    const std::vector<std::pair<std::string, std::string>> & get_replaces() const noexcept { return replaces; }
    void set_replaces(std::vector<std::pair<std::string, std::string>> && replaces) { this->replaces = replaces; }

private:
    libdnf5::transaction::TransactionItemAction action;
    libdnf5::transaction::TransactionItemReason reason;
    std::string module_name;
    std::string module_stream;
    std::vector<std::pair<std::string, std::string>> replaces;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPER_DBUS_TRANSACTION_MODULE_WRAPPER_HPP
