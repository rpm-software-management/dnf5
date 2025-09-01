// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_TRANSACTION_ITEM_ACTION_HPP
#define LIBDNF5_TRANSACTION_ITEM_ACTION_HPP

#include "transaction_item_errors.hpp"

#include "libdnf5/defs.h"

#include <string>


namespace libdnf5::transaction {

// Any time you add a new action, change functions that resolve reasons,
// because removed items (RPMs) must be excluded from reason resolution:
// * Package.cpp - Package::resolveTransactionItemReason
// TODO(jmracek) Copy enum to SWDB including their string definitions and already removed values to keep compatibility
// Then transform old database data to transform stored reverse action to REPLACED action and remove all unused action from code
enum class TransactionItemAction : int {
    INSTALL = 1,
    UPGRADE = 2,
    DOWNGRADE = 3,
    REINSTALL = 4,
    REMOVE = 5,
    REPLACED = 6,       // a package that is being replaced by another package (this one is leaving the system)
    REASON_CHANGE = 7,  // a package is being kept on the system but its reason is changing
    ENABLE = 8,         // module enable
    DISABLE = 9,        // module disable
    RESET = 10,         // module reset
    SWITCH = 11         // module switch
};


LIBDNF_API std::string transaction_item_action_to_string(TransactionItemAction action);
LIBDNF_API TransactionItemAction transaction_item_action_from_string(const std::string & action);

LIBDNF_API std::string transaction_item_action_to_letter(TransactionItemAction action);

LIBDNF_API bool transaction_item_action_is_inbound(TransactionItemAction action);
LIBDNF_API bool transaction_item_action_is_outbound(TransactionItemAction action);

}  // namespace libdnf5::transaction

/*
Install
-------
* Command example: dnf install bash
* Old package(s): (none)
* New package(s): bash-4.4.12-5
* -> new TransactionItem: item="bash-4.4.12-5", action=INSTALL, reason=<new>, replaced_by=NULL

Downgrade
---------
* Command example: dnf downgrade bash
* Old package(s): bash-4.4.12-5
* New package(s): bash-4.4.12-4
* -> new TransactionItem: item="bash-4.4.12-5", action=DOWNGRADE, reason=<inherited>,
replaced_by=NULL
* -> new TransactionItem: item="bash-4.4.12-4", action=DOWNGRADED, reason=<inherited>,
replaced_by=<id of bash-4.4.12-5 transaction item>

Obsolete
--------
* Command example: dnf upgrade
* Old package(s): sysvinit-2.88-9
* New package(s): systemd-233-6
* -> new TransactionItem: item="systemd-233-6", action=OBSOLETE, reason=<inherited>,
replaced_by=NULL
* -> new TransactionItem: item="sysvinit-2.88-9", action=OBSOLETED, reason=<inherited>,
replaced_by=<id of systemd-233-6 transaction item>

Obsolete & Upgrade
------------------
* Command example: dnf upgrade
* Old package(s): systemd-233-5, sysvinit-2.88-9
* New package(s): systemd-233-6 (introducing Obsoletes: sysvinit)
* -> new TransactionItem: item="systemd-233-6", action=UPGRADE, reason=<inherited>, replaced_by=NULL
* -> new TransactionItem: item="systemd-233-5", action=UPGRADED, reason=<inherited>, replaced_by=<id
of systemd-233-6 transaction item>
* -> new TransactionItem: item="sysvinit-2.88-9", action=OBSOLETED, reason=<inherited>,
replaced_by=<id of systemd-233-6 transaction item>

Upgrade
-------
* Command example: dnf upgrade
* Old package(s): bash-4.4.12-4
* New package(s): bash-4.4.12-5
* -> new TransactionItem: item="bash-4.4.12-5", action=UPGRADE, reason=<inherited>, replaced_by=NULL
* -> new TransactionItem: item="bash-4.4.12-4", action=UPGRADED, reason=<inherited>, replaced_by=<id
of bash-4.4.12-4 transaction item>

Remove
------
* Command example: dnf remove bash
* Old package(s): bash-4.4.12-5
* New package(s): (none)
* -> new TransactionItem: item="bash-4.4.12-5", action=REMOVED, reason=<new>, replaced_by=NULL

Reinstall
---------
* Command example: dnf reinstall bash
* Old package(s): bash-4.4.12-5
* New package(s): bash-4.4.12-5
* -> new TransactionItem: item="bash-4.4.12-5", action=REINSTALL, reason=<inherited>,
replaced_by=NULL

Reason Change
-------------
* Command example: dnf mark install bash
* Old package(s): bash-4.4.12-5
* New package(s): bash-4.4.12-5
* -> new TransactionItem: item="bash-4.4.12-5", action=REASON_CHANGE, reason=<new>,

Reasons:
* new = a brand new reason why a package was installed or removed
* inherited = a package was installed in the past, reuse it's reason in existing transaction
*/

#endif  // LIBDNF5_TRANSACTION_TYPES_HPP
