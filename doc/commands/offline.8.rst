..
    Copyright Contributors to the libdnf project.
    SPDX-License-Identifier: GPL-2.0-or-later

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

.. _offline_command_ref-label:

################
 Offline Command
################

Synopsis
========

``dnf5 offline <subcommand> [options]``


Description
===========

The ``offline`` command is used to manage "offline" transactions---transactions that run when the system is booted into a minimal environment. Running a transaction in this stripped-down environment can be safer than running it when the system is booted normally since the transaction is less likely to interfere with running processes.

Offline transactions can be initiated by specifying the ``--offline`` flag on any operation (``install``, ``upgrade``, ``distro-sync``, etc.), or via ``dnf5 system-upgrade download``. Once an offline transaction is initiated, run ``dnf5 offline reboot`` to reboot and begin the transaction.

Data for offline transactions is stored under the "system state" directory at ``/usr/lib/sysimage/libdnf5/offline``.


Subcommands
===========

``clean``
    | Removes any stored offline transaction and deletes cached package files.

``log``
    | Used to see a list of boots during which an offline transaction was attempted, or show the logs from an attempted offline transaction. The logs for one of the boots can be shown by specifying one of the numbers in the first column with the ``--number`` argument. Negative numbers can be used to number the boots from last to first. For example, ``log --number=-1`` can be used to see the logs for the last offline transaction.

``reboot``
    | Prepares the system to perform the offline transaction and reboots to start the transaction. This command can only be run after an offline transaction is initiated (e.g. by ``dnf5 system-upgrade download``).

``status``
    | Shows the status of the current offline transaction.

``_execute``
    | Execute the transaction in the offline environment.

    .. warning::
       For internal use only. Not intended to be run by the user.


Options
=======

``--number=<boot number>``
    | Show the log specified by the number. Run ``dnf5 offline log`` with ``--number`` first to get a list of logs to choose from.
    | Used with the ``log`` subcommand.

``--poweroff``
    | The system will power off after the transaction is completed instead of restarting. If the transaction failed, the system will reboot instead of powering off even with this flag.
    | Used with the ``reboot`` subcommand.


Environment
===========

``DNF_SYSTEM_UPGRADE_NO_REBOOT``
    If set, the system won't be rebooted or powered off by DNF5 when the normal work flow would do so.


Examples
========

``dnf5 install --offline hello``
    | Prepares the installation of the ``hello`` package as an offline transaction.

``dnf5 offline status``
    | Shows the status of the current offline transaction.

``dnf5 offline reboot --poweroff``
    | Reboot and run the offline transaction, then power the system off after the transaction is complete.

``dnf5 offline log``
    | List boots during which an offline transaction was attempted.

``dnf5 offline log --number=-1``
    | View the log from the latest boot during which an offline transaction was attempted.


See Also
========

    | :manpage:`dnf5-system-upgrade(8)`, :ref:`System-upgrade command <system-upgrade_command_ref-label>`
    | https://www.freedesktop.org/wiki/Software/systemd/SystemUpdates
