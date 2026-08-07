..
    Copyright Contributors to the DNF5 project.
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

.. _status_command_ref-label:

################
 Status Command
################

Synopsis
========

``dnf5 status``


Description
===========

Display summary statistics about the system's package management state.
Shows counts of installed and available groups, environments and packages,
total installed size, number of enabled repositories, and the time of the
last transaction.


Examples
========

``dnf5 status``
    | Display overall system status.


See Also
========

    | :manpage:`dnf5-comps(7)`, :ref:`Comps groups and environments <comps_misc_ref-label>`
