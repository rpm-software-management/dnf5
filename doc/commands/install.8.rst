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

.. _install_command_ref-label:

################
 Install Command
################

Synopsis
========

``dnf5 install [options] <package-spec>...``


Description
===========

The ``install`` command in ``DNF5`` is used for installing packages. It makes sure that
all given packages defined in ``package-spec`` arguments and their dependencies are installed
on the system. Environments can be installed with ``@environment-id`` as ``<package-spec>``.


Options
=======

``--allowerasing``
    | Allow erasing of installed packages to resolve any potential dependency problems.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.

``--skip-unavailable``
    | Allow skipping packages that are not available in repositories. All available packages will be installed.

``--allow-downgrade``
    | Enable downgrade of dependencies when resolving the requested operation.

``--no-allow-downgrade``
    | Disable downgrade of dependencies when resolving the requested operation.

``--downloadonly``
    | Download the resolved package set without executing an RPM transaction.

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

``dnf5 install tito``
    | Install the ``tito`` package.

``dnf5 install ~/Downloads/tito-0.6.21-1.fc36.noarch.rpm``
    | Install the local rpm file from the given location.

``dnf5 install tito-0.6.21-1.fc36``
    | Install the ``tito`` package in defined version.
    | If the package is already installed, it will automatically try to downgrade or upgrade to the given version.

``dnf5 install --advisory=FEDORA-2022-07aa56297a \*``
    | Install all the packages that belong to the ``FEDORA-2022-07aa56297a`` advisory.


See Also
========

    | :manpage:`dnf5-advisory(8)`, :ref:`Advisory command <advisory_command_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
