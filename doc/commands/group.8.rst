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

.. _group_command_ref-label:

##############
 Group Command
##############

Synopsis
========

``dnf5 group {list|info} [options] [<group-spec>...]``

``dnf5 group {install|remove|upgrade} [options] <group-spec|environment-spec>...``


Description
===========

The ``group`` command in ``DNF5`` offers several queries for getting information about groups, packages
related to them and it is also used for groups installation.
To query environments use separate ``environment`` command.
Note: ``dnf-4`` listed both environments and groups with the ``group`` command.

Optional ``group-spec`` arguments can be passed to filter groups with given IDs or names. The install, remove and
upgrade commands take both ``group-spec`` and ``environment-spec``.


Subcommands
===========

``list``
    List all matching groups, either among installed or available groups. If
    nothing is specified, list all known groups. ``--installed`` and ``--available``
    options narrow down the requested list. If ``--hidden`` option is used, also
    hidden groups are included in the list.

``info``
    Print detailed information about groups.
    The command accepts the same options as the ``list`` subcommand.

``install``
    Mark the specified environments or groups installed and install groups and packages they
    contain.

    If an environments or group is already (partially) installed, the command  installs the
    missing groups and packages it contains.

    In case both groups and environments match, only groups are affected.

    If the ``--with-optional`` option is used, also include ``Optional`` packages of groups. By default,
    all ``Mandatory`` and ``Default`` packages will be installed whenever possible. ``Conditional``
    packages are installed if they meet their requirement. This can be configured by
    :manpage:`dnf5.conf(5)`, :ref:`group_package_types <group_package_types_options-label>`.

    If the ``--no-packages`` option is used, no packages will be installed by this command.
    Only currently installed group packages are considered to be installed with the groups.

``remove``
    Mark the specified environments or groups removed and remove groups and packages they contain
    unless they belong to another installed environment or group, were installed explicitly by the
    user or (in case of packages) were installed as a dependency.

    In case both groups and environments match, only groups are affected.

    If the ``--no-packages`` option is used, no packages will be removed by this command.

``upgrade``
    Upgrade a definition of the specified environments and groups and the groups and packages they
    contain. If new groups or packages have been added to the current definitions since the
    environments or groups were installed, the new groups and packages will be installed. Likewise,
    if some groups or packages have been removed from the definition, they will be removed
    unless they were installed for a different reason (belong to another installed environment or
    group, were installed explicitly by the user or were installed as a dependency).

    In case both groups and environments match, only groups are affected.

Options for ``list`` and ``info``
=================================

``--available``
    | Show only available groups. Those which are not installed, but known to ``DNF5``.

``--installed``
    | Show only installed groups.

``--hidden``
    | Show also hidden groups.

``--contains-pkgs=PACKAGE_NAME,...``
    | Show only groups containing packages with specified names. List option, supports globs.


Options for ``install``, ``remove`` and ``upgrade``
===================================================

``--with-optional``
    | Include optional packages from the groups.
    | Used with ``install`` command.

``--no-packages``
    | Operate exclusively on the environments and groups without manipulating any packages.
    | Used with ``install`` and ``remove`` commands.

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.
    | Used with ``install`` and ``upgrade`` commands.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.
    | Used with ``install`` command.

``--skip-unavailable``
    | Allow skipping packages that are not possible to install or upgrade.
    | Used with ``install`` and ``upgrade`` commands.

``--allow-downgrade``
    | Enable downgrade of dependencies when resolving the requested operation.
    | Used with ``install`` and ``upgrade`` commands.

``--no-allow-downgrade``
    | Disable downgrade of dependencies when resolving the requested operation.
    | Used with ``install`` and ``upgrade`` commands.

``--downloadonly``
    | Download the resolved package set without executing an RPM transaction.
    | Used with ``install`` and ``upgrade`` commands.

.. include:: ../_shared/options/transaction.rst


Examples
========

``dnf5 group list --hidden``
    | Show list of all groups, including hidden ones.

``dnf5 group info *xfce*``
    | Show detailed info about all groups related to ``Xfce``.

``dnf5 group install mysql --with-optional``
    | Install the ``mysql`` group including optional packages.

``dnf5 group upgrade mysql``
    | Bring packages of the ``mysql`` group to compliance with a current
    | definition of the group.

See Also
========

    | :manpage:`dnf5-comps(7)`, :ref:`Comps groups and environments <comps_misc_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
