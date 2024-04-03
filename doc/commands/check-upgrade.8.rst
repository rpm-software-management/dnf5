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

.. _check-upgrade_command_ref-label:

######################
 Check-Upgrade Command
######################

Synopsis
========

``dnf5 check-upgrade [options] [<package-spec>...]``


Description
===========

Non-interactively checks for available updates of specified packages. If no ``<package-spec>``
is provided, it checks for updates for the entire system.

``DNF5`` will exit with code `100`` if updates are available and list them; `0` if no updates
are available.


Options
=======

``--changelogs``
    | Print the package changelogs.

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

``dnf5 check-upgrade``
    | Print a list of packages that have updates available.

``dnf5 check-upgrade --changelogs``
    | Print changelogs for all packages with pending updates.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
