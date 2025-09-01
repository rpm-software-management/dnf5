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

.. _changelog_plugin_ref-label:

##################
 Changelog Command
##################

Synopsis
========

``dnf5 changelog [options] [<package-spec-NI>...]``


Description
===========

Show package changelogs.


Options
=======

``--since=DATE``
    | Show only changelog entries since ``DATE``.
    | `YYYY-MM-DD` date format is expected.

``--count=VALUE``
    | Limit the number of changelog entries shown per package to ``VALUE``.

``--upgrades``
    |  Show only new changelog entries for packages that provide upgrades for already installed packages.


Examples
========

``dnf5 changelog --since=2023-04-01``
    | Display changelog entries since April 1, 2023 for all packages.

``dnf5 changelog --count=5 bash``
    | Display the 3 latest changelogs for the ``bash`` package.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
