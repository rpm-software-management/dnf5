..
    Copyright Contributors to the DNF5 project.
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

.. _installonly_lock_command_ref-label:

#########################
 Installonly Lock Command
#########################

Synopsis
========

``dnf5 installonly-lock <subcommand> [package-spec-NPFB]...``


Description
===========

The ``installonly-lock`` command marks installed installonly packages so they are
not automatically removed when ``installonly_limit`` is exceeded.

Locked packages are still kept when a newer installonly version is installed and
``limit_installonly_packages`` would otherwise erase the oldest versions. Manual
removal with ``dnf5 remove`` is not blocked.


Subcommands
===========

``add``
    | Lock installed installonly packages matching the spec against automatic removal.

``delete``
    | Remove the installonly lock from matching installed packages.

``list``
    | List currently locked installonly packages.


Examples
========

``dnf5 installonly-lock add kernel-6.5.0-1.fc39``
    | Keep this kernel version even when installonly_limit would remove it.

``dnf5 installonly-lock list``
    | Show locked installonly packages.

``dnf5 installonly-lock delete kernel-6.5.0-1.fc39``
    | Allow automatic removal of this kernel version again.


See Also
========

    | :manpage:`dnf5.conf(5)`, :ref:`Installonlypkgs Options <installonlypkgs_options-label>`
    | :manpage:`dnf5-system-state(7)`, :ref:`System State <systemstate_misc_ref-label>`
