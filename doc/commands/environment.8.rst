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

..
    # TODO(jkolarik): Still some subcommands missing in the upstream (install, ...)

.. _environment_command_ref-label:

####################
 Environment Command
####################

Synopsis
========

``dnf5 environment <subcommand> [options] [<environment-spec>...]``


Description
===========

The ``environment`` command in ``DNF5`` offers several queries for getting information
about environments and groups related to them. You can install environments
with the ``install`` command as ``install @environment-id``.

Optional ``environment-spec`` arguments could be passed to filter only environments with given names.


Subcommands
===========

``list``
    | List available environments.

``info``
    | Print details about environments.


Options
=======

``--available``
    | Show only available environments. Those which are not installed, but known to ``DNF5``.

``--installed``
    | Show only installed environments.


Examples
========

``dnf5 environment list``
    | Show list of all environments.

``dnf5 environment info "KDE Plasma Workspaces"``
    | Show detailed info about the ``KDE`` environment.


See Also
========

    | :manpage:`dnf5-comps(7)`, :ref:`Comps groups and environments <comps_misc_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
