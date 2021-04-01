/*
Copyright (C) 2020 Red Hat, Inc.

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

#ifndef DNFDAEMON_SERVER_SERVICES_RPM_TRANSACTION_HPP
#define DNFDAEMON_SERVER_SERVICES_RPM_TRANSACTION_HPP

#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/transaction.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon {

class RpmTransactionItem : public libdnf::rpm::TransactionItem {
public:
    enum class Actions { INSTALL, ERASE, UPGRADE, DOWNGRADE, REINSTALL, CLEANUP };

    RpmTransactionItem(libdnf::rpm::Package pkg, Actions action) : TransactionItem(pkg), action(action) {}
    Actions get_action() const noexcept { return action; }

private:
    Actions action;
};

}  // namespace dnfdaemon

#endif
