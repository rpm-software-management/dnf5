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

If terminal is available, list of the packages is colored, packages available for reinstall are
(by default) colored with bold green and packages available for upgrade with bold blue. This
behavior can be adjusted in :manpage:`dnf5.conf(5)` via `color_list_available_upgrade` and
`color_list_available_reinstall` options.


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

``--json``
    | Request JSON output format.

Examples
========

``dnf5 list``
    | List installed and available packages.

``dnf5 list --available --showduplicates``
    | List all available packages, including all available versions.

``dnf5 list kernel*``
    | List installed and available packages whose names start with ``kernel``.

JSON Output
===========

* ``dnf5 list --json``

The command returns a JSON object with the following structure:

- keys represent a section of the “pretty” CLI output
- values represent the packages in that section as an array of objects with the following structure:
    - ``name`` (string): package name
    - ``arch`` (string): package architecture
    - ``evr`` (string): available update version
    - ``repository`` (string): repository ID from which the update is available
    - ``obsoletes`` (array): (only for the “Obsoleting packages” section) list of the packages that
        obsolete this package (they have the same structure as above, omitting ``obsoletes``)

See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
