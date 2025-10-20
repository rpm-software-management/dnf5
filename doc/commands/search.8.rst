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

.. _search_command_ref-label:

###############
 Search Command
###############

Synopsis
========

``dnf5 search [options] <pattern>...``


Description
===========

The ``search`` command in ``DNF5`` is used for searching packages by matching
given keywords from the user against various metadata.

By default the command searches for all requested keys (AND operation) in
`Name` or `Summary` fields from the RPM package metadata. Users can limit
the search to specific fields using ``--name`` or ``--summary``
options. Matching is case-insensitive and globs are supported.


Options
=======

``--all``
    | Search patterns also inside `Description` and `URL` fields.
    | By applying this option the search lists packages that match at least one of the keys (OR operation).
    | Cannot be used with ``--name`` or ``--summary``.

``--name``
    | Limit the search to the `Name` field only.
    | Cannot be used with ``--all`` or ``--summary``.

``--summary``
    | Limit the search to the `Summary` field only.
    | Cannot be used with ``--all`` or ``--name``.

``--showduplicates``
    | Show all versions of packages, not only the latest ones.


Examples
========

``dnf5 search kernel``
    | Search ``kernel`` keyword inside `Name` or `Summary` package metadata fields.

``dnf5 search rpm dbus``
    | Search packages having both ``rpm`` and ``dbus`` keywords inside `Name` or `Summary` fields.

``dnf5 search --all fedora gnome kde``
    | Search packages having any of given keywords in `Name`, `Summary`, `Description` or `URL` fields.

``dnf5 search --name firefox``
    | Search ``firefox`` keyword only in the `Name` field of packages.

``dnf5 search --summary browser``
    | Search ``browser`` keyword only in the `Summary` field of packages.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
