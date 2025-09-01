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

.. _check-upgrade_command_ref-label:

######################
 Check-Upgrade Command
######################

Synopsis
========

``dnf5 check-upgrade [options] [<package-spec-N>...]``


Description
===========

Non-interactively checks for available updates of specified packages. If no ``<package-spec-N>``
is provided, it checks for updates for the entire system.

``DNF5`` will exit with code `100`` if updates are available and list them; `0` if no updates
are available.


Options
=======

``--changelogs``
    | Print the package changelogs.

.. include:: ../_shared/options/advisories.rst

.. include:: ../_shared/options/advisory-severities.rst

.. include:: ../_shared/options/bzs.rst

.. include:: ../_shared/options/cves.rst

.. include:: ../_shared/options/security.rst

.. include:: ../_shared/options/bugfix.rst

.. include:: ../_shared/options/enhancement.rst

.. include:: ../_shared/options/newpackage.rst

``--minimal``
    | Reports the lowest versions of packages that fix advisories of type bugfix, enhancement, security, or
    | newpackage. In case that any option limiting advisories is used it reports the lowest versions of packages
    | that fix advisories matching selected advisory properties"


Examples
========

``dnf5 check-upgrade``
    | Print a list of packages that have updates available.

``dnf5 check-upgrade --changelogs``
    | Print changelogs for all packages with pending updates.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
