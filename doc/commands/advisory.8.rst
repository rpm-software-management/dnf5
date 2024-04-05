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

.. _advisory_command_ref-label:

#################
 Advisory Command
#################

Synopsis
========

``dnf5 advisory <subcommand> [options] [<advisory-spec>...]``


Description
===========

The ``advisory`` command in ``DNF5`` offers several queries for getting information about
advisories and packages related to them.

Optional ``advisory-spec`` arguments could be passed to filter only advisories with given names.


Subcommands
===========

``list``
    | List available advisories.

``info``
    | Print details about advisories.

``summary``
    | Print a summary of advisories.


Options
=======

``--all``
    | Show advisories containing any version of installed packages.

``--available``
    | Show advisories containing newer versions of installed packages. This is the default behavior.

``--installed``
    | Show advisories containing equal and older versions of installed packages.

``--updates``
    | Show advisories containing newer versions of installed packages for which a newer version is available.

``--contains-pkgs=PACKAGE_NAME,...``
    | Show only advisories containing packages with specified names.
    | This is a list option.
    | Only installed packages are matched. Globs are supported.

``--security``
    | Consider only content contained in security advisories.

``--bugfix``
    | Consider only content contained in bugfix advisories.

``--enhancement``
    | Consider only content contained in enhancement advisories.

``--newpackage``
    | Consider only content contained in newpackage advisories.

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

``--with-bz``
    | Show only advisories referencing a Bugzilla ticket.

``--with-cve``
    | Show only advisories referencing a CVE ticket.


Examples
========

``dnf5 advisory info FEDORA-2022-07aa56297a``
    | Show detailed info about advisory with given name.

``dnf5 advisory summary --contains-pkgs=kernel,kernel-core --with-bz``
    | Show a summary of advisories containing ``kernel`` or ``kernel-core`` packages and referencing any Bugzilla ticket.

``dnf5 advisory list --security --advisory-severities=important``
    | Show a list of security advisories or advisories with ``important`` severity.
