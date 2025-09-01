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

.. _upgrade_command_ref-label:

################
 Upgrade Command
################

Synopsis
========

``dnf5 upgrade [options] [<package-spec-NPFB>|@<group-spec>|@<environment-spec>...]``


Description
===========

The ``upgrade`` command in ``DNF5`` is used for upgrading installed packages, groups or
environments to newer available version.

Since groups and environments are not versioned the upgrade basically means a synchronization
with the currently available definition. In addition group upgrade also upgrades all packages
the group contains and environment upgrade also upgrades all groups the environment contains.



Options
=======

``--minimal``
    | Upgrade packages only to the lowest available versions that fix advisories of type bugfix, enhancement, security, or
    | newpackage. In case that any option limiting advisories is used it upgrades packages only to the lowest versions
    | that fix advisories matching selected advisory properties

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

``--skip-unavailable``
    | Allow skipping packages that are not possible to upgrade. All remaining packages will be upgraded.

``--allow-downgrade``
    | Enable downgrade of dependencies when resolving the requested operation.

``--no-allow-downgrade``
    | Disable downgrade of dependencies when resolving the requested operation.

``--destdir=<path>``
    | Set directory used for downloading packages to. Default location is to the current working directory.
    | Automatically sets the ``downloadonly`` option.

.. include:: ../_shared/options/installed-from-repo.rst

.. include:: ../_shared/options/from-repo.rst

``--downloadonly``
    | Only download packages for transaction.

.. include:: ../_shared/options/transaction.rst

.. include:: ../_shared/options/advisories.rst

.. include:: ../_shared/options/advisory-severities.rst

.. include:: ../_shared/options/bzs.rst

.. include:: ../_shared/options/cves.rst

.. include:: ../_shared/options/security.rst

.. include:: ../_shared/options/bugfix.rst

.. include:: ../_shared/options/enhancement.rst

.. include:: ../_shared/options/newpackage.rst


Examples
========

``dnf5 upgrade``
    | Upgrade all installed packages to the newest available version.

``dnf5 upgrade tito``
    | Upgrade the ``tito`` package.


See Also
========

    | :manpage:`dnf5-advisory(8)`, :ref:`Advisory command <advisory_command_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
