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

.. include:: ../_shared/options/security.rst

.. include:: ../_shared/options/bugfix.rst

.. include:: ../_shared/options/enhancement.rst

.. include:: ../_shared/options/newpackage.rst

.. include:: ../_shared/options/advisory-severities.rst

.. include:: ../_shared/options/bzs.rst

.. include:: ../_shared/options/cves.rst

``--with-bz``
    | Show only advisories referencing a Bugzilla ticket.

``--with-cve``
    | Show only advisories referencing a CVE ticket.

``--json``
    | Request JSON output format for machine-readable results.
    | Available for ``list`` and ``info`` subcommands only.


Examples
========

``dnf5 advisory info FEDORA-2022-07aa56297a``
    | Show detailed info about advisory with given name.

``dnf5 advisory summary --contains-pkgs=kernel,kernel-core --with-bz``
    | Show a summary of advisories containing ``kernel`` or ``kernel-core`` packages and referencing any Bugzilla ticket.

``dnf5 advisory list --security --advisory-severities=important``
    | Show a list of security advisories or advisories with ``important`` severity.

``dnf5 advisory list --json``
    | List all advisories in JSON format for programmatic processing.

``dnf5 advisory list --json --with-cve``
    | List advisories with CVE references in JSON format (extended format).

``dnf5 advisory info FEDORA-2022-07aa56297a --json``
    | Show detailed info about advisory in JSON format.


JSON Output
===========

* ``dnf5 advisory list --json``

The command returns a JSON array, each element describing one advisory.

**Basic format** (without ``--with-cve`` or ``--with-bz``):

- ``name`` (string) - Advisory identifier.
- ``type`` (string) - Advisory type (security, bugfix, enhancement).
- ``severity`` (string) - Advisory severity level.
- ``nevra`` (string) - Package name-epoch:version-release.architecture.
- ``buildtime`` (integer) - Advisory build time, UNIX time.

**Extended format** (with ``--with-cve`` or ``--with-bz``):

- ``advisory_name`` (string) - Advisory identifier.
- ``advisory_type`` (string) - Advisory type (security, bugfix, enhancement).
- ``advisory_severity`` (string) - Advisory severity level.
- ``advisory_buildtime`` (integer) - Advisory build time, UNIX time.
- ``nevra`` (string) - Package name-epoch:version-release.architecture.
- ``references`` (array) - List of references (CVE/Bugzilla). Each reference contains:

  - ``reference_id`` (string) - Reference identifier (e.g., CVE-2024-1234).
  - ``reference_type`` (string) - Reference type (cve or bugzilla).

* ``dnf5 advisory info --json``

The command returns a JSON array, each element containing detailed advisory information.
Each advisory object contains the following fields:

- ``Name`` (string) - Advisory name/identifier.
- ``Title`` (string) - Advisory title.
- ``Type`` (string) - Advisory type (security, bugfix, enhancement).
- ``Severity`` (string) - Advisory severity level.
- ``Status`` (string) - Advisory status.
- ``Vendor`` (string) - Advisory vendor.
- ``Issued`` (integer) - Advisory issue time, UNIX time.
- ``Description`` (string) - Detailed advisory description.
- ``Message`` (string) - Advisory message.
- ``Rights`` (string) - Advisory rights/copyright information.
- ``references`` (array) - List of references (CVE, Bugzilla, etc.). Each reference contains:

  - ``Title`` (string) - Reference title.
  - ``Id`` (string) - Reference identifier (e.g., CVE-2024-1234).
  - ``Type`` (string) - Reference type (cve, bugzilla, etc.).
  - ``Url`` (string) - Reference URL.

- ``collections`` (object) - Package and module collections affected by the advisory:

  - ``packages`` (array) - List of affected package NEVRAs (only present if packages exist).
  - ``modules`` (array) - List of affected module NSVCAs (only present if modules exist).

For empty results:

- ``dnf5 advisory list --json`` returns ``[]`` (empty array).
- ``dnf5 advisory info --json`` returns ``[]`` (empty array).
