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

.. _mark_command_ref-label:

#############
 Mark Command
#############

Synopsis
========

``dnf5 mark <subcommand> [global options] [<group-id>] <package-spec-NPFB>...``


Description
===========

The ``mark`` command in ``DNF5`` is used to change reason of installed packages
defined in ``package-spec-NPFB`` arguments.


Subcommands
===========

``user``
    | Mark the package as user-installed.

    This can be useful if any package was installed as a dependency and is desired
    to stay on the system when ``remove`` command along with ``clean_requirements_on_remove``
    configuration option set to ``True`` is executed.

``dependency``
    | Mark the package as a dependency.

    This can be useful if you as the user don't need a specific package. The package stays
    installed on the system, but will be removed when ``remove`` command along with
    ``clean_requirements_on_remove`` configuration option set to ``True`` is executed.

    You should use this operation instead of ``remove`` command if you're not sure whether
    the package is a requirement of other user installed package on the system.

``weak``
    | Mark the package as a weak dependency.

..
    # TODO(jkolarik): weak - What is the purpose of doing this?

``group``
    | Mark the package as installed by the group defined in ``group-id`` argument.

    This can be useful if any package was installed as a dependency or the user and
    is desired to be protected and handled as a group member like during ``group remove`` command.


Options
=======

``--skip-unavailable``
    | Allow skipping packages that are not installed on the system. All remaining installed packages will be marked.

.. include:: ../_shared/options/transaction.rst


Examples
========

``dnf5 mark user fuse-devel``
    | Mark the ``fuse-devel`` package as user-installed.

``dnf5 mark group xfce-desktop vim-enhanced``
    | Mark the ``vim-enhanced`` package as installed by the ``xfce-desktop`` group.


See Also
========

    | :manpage:`dnf5-comps(7)`, :ref:`Comps groups and environments <comps_misc_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
