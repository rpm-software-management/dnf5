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

.. _leaves_command_ref-label:

################
 Leaves Command
################

Synopsis
========

``dnf5 leaves``


Description
===========

The ``leaves`` command in ``DNF5`` is used to list all leaf packages.
Leaf packages are installed packages that are not required as a dependency of another installed package.
However, two or more installed packages might depend on each other in a dependency cycle. Packages
in such cycles that are not required by any other installed package are also leaf.
Packages in such cycles form a group of leaf packages.

Packages in the output list are sorted by group and the first package in the group is preceded by a ``-`` character.


Options
=======

Does not implement options. But it takes into account the ``install_weak_deps`` setting.
If ``install_weak_deps`` is set to ``false``, weak dependencies are ignored during the calculation of the set of leaf packages.


Why is this useful?
===================

The list gives you a nice overview of what is installed on your system without flooding you with anything required by the packages already shown.
The following list of arguments basically says the same thing in different ways:

* All the packages on this list is either needed by you, other users of the system or not needed at all -- if it was required by another installed package it would not be on the list.
* If you want to uninstall anything from your system (without breaking dependencies) it must involve at least one package on this list.
* If there is anything installed on the system which is not needed it must be on this list -- otherwise it would be required as a dependency by another package.
