/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP
#define DNFDAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP

#include <vector>

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf/transaction/transaction_item_action.hpp>

using namespace libdnf::transaction;

namespace dnfdaemon::client {

class DbusPackageWrapper {

public:
    DbusPackageWrapper(dnfdaemon::DbusTransactionItem transactionitem);

    uint64_t get_size() const { return std::get<7>(ti); }
    std::string get_full_nevra() const { return full_nevra; }
    std::string get_repo_id() const { return std::get<6>(ti); }

private:
    dnfdaemon::DbusTransactionItem ti;
    std::string full_nevra;
};

}  // namespace dnfdaemon::client

#endif  // DNFDAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP
