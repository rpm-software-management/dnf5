/*
 * Copyright (C) 2017-2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef LIBDNF_TRANSACTION_TYPES_HPP
#define LIBDNF_TRANSACTION_TYPES_HPP

#include "transaction_item_reason.hpp"

namespace libdnf {


using TransactionItemReason = libdnf::transaction::TransactionItemReason;


enum class TransactionState : int {
    UNKNOWN = 0,
    DONE = 1,
    ERROR = 2
};

enum class TransactionItemState : int {
    UNKNOWN = 0,  // default state, must be changed before save
    DONE = 1,
    ERROR = 2
};

enum class ItemType : int { UNKNOWN = 0, RPM = 1, GROUP = 2, ENVIRONMENT = 3 };

// Any time you add a new action, change functions that resolve reasons,
// because removed items (RPMs) must be excluded from reason resolution:
// * RPMItem.cpp - RPMItem::resolveTransactionItemReason
enum class TransactionItemAction : int {
    INSTALL = 1,       // a new package that was installed on the system
    DOWNGRADE = 2,     // an older package version that replaced previously installed version
    DOWNGRADED = 3,    // an original package version that was replaced
    OBSOLETE = 4,      //
    OBSOLETED = 5,     //
    UPGRADE = 6,       //
    UPGRADED = 7,      //
    REMOVE = 8,        // a package that was removed from the system
    REINSTALL = 9,     // a package that was reinstalled with the identical version
    REINSTALLED = 10,  // a package that was reinstalled with the identical version (old repo, for example)
    REASON_CHANGE = 11 // a package was kept on the system but it's reason has changed
};

} // namespace libdnf
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
* inherited = a package was installed in the past, re-use it's reason in existing transaction
*/

#endif // LIBDNF_TRANSACTION_TYPES_HPP
