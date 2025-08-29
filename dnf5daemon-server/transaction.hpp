// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_SERVER_SERVICES_RPM_TRANSACTION_HPP
#define DNF5DAEMON_SERVER_SERVICES_RPM_TRANSACTION_HPP

#include <libdnf5/base/transaction_package.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon {

enum class RpmTransactionItemActions { INSTALL, ERASE, UPGRADE, DOWNGRADE, REINSTALL, CLEANUP };

RpmTransactionItemActions transaction_package_to_action(const libdnf5::base::TransactionPackage & tspkg);

}  // namespace dnfdaemon

#endif
