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

.. _comps_misc_ref-label:

##############################
 Comps Groups And Environments
##############################

Description
===========

Comps files are used for grouping of packages into functional groups. They are stored
in repository metadata files under the ``comps.xml`` filename.

There are two types of structures that can be used for grouping. The first is a
`group` which is composed of lists of packages. The second one is an `environment`
that is composed from the groups.

Each `environment` is made of mandatory and optional groups. All mandatory groups have
to be installed, so the `environment` is marked as `installed`. Optional groups are
not installed by default, they have to be added using the ``--with-optional`` argument.

In the `group`, there are four levels of packages:

`mandatory`
    | These are the essential packages for the functionality of the group.
    | These have to be installed for the group to be considered ``installed``.

`default`
    | These are packages installed together with mandatory packages.
    | They can be excluded, f.e. using the ``--exclude=PACKAGE-SPEC-N,...`` argument.

`optional`
    | These packages are not installed by default.
    | They can be included using the ``--with-optional`` argument.

`conditional`
    | These packages are brought in the transaction if their required package is to be installed.

For commands operating with groups and environments, see references below.


See Also
========

    | :manpage:`dnf5-group(8)`, :ref:`Group command <group_command_ref-label>`
    | :manpage:`dnf5-environment(8)`, :ref:`Environment command <environment_command_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
