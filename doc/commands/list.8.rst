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

.. _list_command_ref-label:

#############
 List Command
#############

Synopsis
========

``dnf5 list [options] [<package-spec-NI>...]``


Description
===========

Prints lists of packages based on the provided parameters.


Options
=======

``--showduplicates``
    | Show all versions of the packages, not only the latest one.

.. include:: ../_shared/options/installed-from-repo.rst

``--installed``
    | List only installed packages.

``--available``
    | List only available packages.

``--extras``
    | List only extras: packages installed on the system that are not available in any known repository.

``--obsoletes``
    | List only packages installed on the system that are obsoleted by packages in any known repository.

``--recent``
    | List only packages recently added to the repositories.

``--upgrades``
    | List only available upgrades for installed packages.

``--autoremove``
    | List only packages that will be removed by the :ref:`autoremove command <autoremove_command_ref-label>`.


Examples
========

``dnf5 list``
    | List installed and available packages.

``dnf5 list --available --showduplicates``
    | List all available packages, including all available versions.

``dnf5 list kernel*``
    | List installed and available packages whose names start with ``kernel``.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
