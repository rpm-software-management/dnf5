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

.. _group_command_ref-label:

##############
 Group Command
##############

Synopsis
========

``dnf5 group <subcommand> [options] [<group-spec>...]``


Description
===========

The ``group`` command in ``DNF5`` offers several queries for getting information about groups, packages
related to them and it is also used for groups installation.

Optional ``group-spec`` arguments could be passed to filter only groups with given names.


Subcommands
===========

``install``
    | Install comps groups, including their packages.

``list``
    | List available groups.

``info``
    | Print details about groups.


Options
=======

``--available``
    | Show only available groups. Those which are not installed, but known to ``DNF5``.

``--installed``
    | Show only installed groups.

``--hidden``
    | Show also hidden groups.

``--with-optional``
    | Used with ``install`` command to include optional packages from the groups.

Examples
========

``dnf5 group list --hidden``
    | Show list of all groups, including hidden ones.

``dnf5 group info *xfce*``
    | Show detailed info about all groups related to ``Xfce``.

``dnf5 group install mysql --with-optional``
    | Install the ``mysql`` group including optional packages.


See Also
========

    | :manpage:`dnf5-comps(7)`, :ref:`Comps groups and environments <comps_misc_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
