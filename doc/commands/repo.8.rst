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

.. _repo_command_ref-label:

#############
 Repo Command
#############

Synopsis
========

``dnf5 repo <subcommand> [options] [<repo-spec>...]``


Description
===========

The ``repo`` command in ``DNF5`` offers several queries for getting information
about repositories configured on the system.


Subcommands
===========

``list``
    | List available repositories.

``info``
    | Show detailed info about repositories.


Options
=======

``--all``
    | Show information about all known repositories.

``--enabled``
    | Show information only about enabled repositories.
    | This is the default behavior.

``--disabled``
    | Show information only about disabled repositories.


Examples
========

``dnf5 repo info --all``
    | Print detailed info about all known repositories.

``dnf5 repo list --disabled *-debuginfo``
    | Print disabled repositories related to debugging.

``dnf5 config-manager setopt repo_id.enabled=0``
    | Persistently disable repository using the config-manager plugin command.
    | See :manpage:`dnf5-config-manager(8)` for more details.
