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

.. _distro-sync_command_ref-label:

####################
 Distro-Sync Command
####################

Synopsis
========

``dnf5 distro-sync [options] [<package-spec>...]``


Description
===========

The ``distro-sync`` command in ``DNF5`` serves to synchronize the installed packages
with their latest available version from any enabled repository. It upgrades, downgrades
or keeps packages as needed.

Optional ``package-spec`` arguments could be passed to select only specific packages to be synced.


Options
=======

``--allowerasing``
    | Allow erasing of installed packages to resolve any potential dependency problems.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.


Examples
========

``dnf5 distro-sync``
    | Sync the whole system to the latest available version of packages.

``dnf5 distro-sync vim``
    | Sync the ``vim`` package to the latest available version.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
