..
    Copyright Contributors to the libdnf project.

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

``dnf5 upgrade [options] [<package-spec>...]``


Description
===========

The ``upgrade`` command in ``DNF5`` is used for upgrading installed packages to the
newer available version.


Options
=======

``--minimal``
    | Update packages only to the lowest available version that provides bug fixes, enhancements, or security fixes.

``--allowerasing``
    | Allow erasing of installed packages to resolve any potential dependency problems.

``--skip-unavailable``
    | Allow skipping packages that are not possible to upgrade. All remaining packages will be upgraded.

``--allow-downgrade``
    | Enable downgrade of dependencies when resolving the requested operation.

``--no-allow-downgrade``
    | Disable downgrade of dependencies when resolving the requested operation.

``--destdir=<path>``
    | Set directory used for downloading packages to. Default location is to the current working directory.
    | Automatically sets the ``downloadonly`` option.

``--downloadonly``
    | Only download packages for transaction.

``--offline``
    | Store the transaction to be performed offline. See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`.

``--advisories=ADVISORY_NAME,...``
    | Consider only content contained in advisories with specified name.
    | This is a list option.
    | Expected values are advisory IDs, e.g. `FEDORA-2201-123`.

``--advisory-severities=ADVISORY_SEVERITY,...``
    | Consider only content contained in advisories with specified severity.
    | This is a list option.
    | Accepted values are: `critical`, `important`, `moderate`, `low`, `none`.

``--bzs=BUGZILLA_ID,...``
    | Consider only content contained in advisories that fix a ticket of given Bugzilla ID.
    | This is a list option.
    | Expected values are numeric IDs, e.g. `123123`.

``--cves=CVE_ID,...``
    | Consider only content contained in advisories that fix a ticket of given CVE (Common Vulnerabilities and Exposures) ID.
    | This is a list option.
    | Expected values are string IDs in CVE format, e.g. `CVE-2201-0123`.

``--security``
    | Consider only content contained in security advisories.

``--bugfix``
    | Consider only content contained in bugfix advisories.

``--enhancement``
    | Consider only content contained in enhancement advisories.

``--newpackage``
    | Consider only content contained in newpackage advisories.


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
