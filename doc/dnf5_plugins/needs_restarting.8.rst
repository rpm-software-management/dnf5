.. Copyright Contributors to the DNF5 project.
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

.. _needs_restarting_plugin_ref-label:

#########################
 Needs-Restarting Command
#########################

Synopsis
========

``dnf5 needs-restarting [--services | -s]``


Description
===========

The ``needs-restarting`` command determines whether the system should be rebooted to fully apply changes from package installations and upgrades. Without any options, ``dnf5 needs-restarting`` will report whether any important packages were installed or upgraded since boot. This set of important packages includes the kernel, systemd, each package listed here: https://access.redhat.com/solutions/27943, and any package marked with a ``reboot_suggested`` advisory.

The ``needs-restarting`` command will exit with code 1 if a reboot is recommended, or, when invoked with ``--services``, if any systemd service needs restarting. If no action is recommended, ``needs-restarting`` will exit with code 0.


Options
=======

``-s, --services``
    | List systemd services that need restarting. If the package that provides the service, or any of its dependencies, have been updated since the service started, then restarting the service will be recommended. Note that this approach is quite aggressive to recommend a restart when one may not be strictly necessary.
