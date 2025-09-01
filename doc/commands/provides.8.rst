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

.. _provides_command_ref-label:

##################
 Provides Command
##################

Synopsis
========

``dnf5 provides [global options] <package-spec-PFB>...``


Description
===========

The ``provides`` command in ``DNF5`` finds the packages providing the given ``<package-spec-PFB>``.


Examples
========

``dnf5 provides tito``
    | Outputs which packages provide the command ``tito``.


``dnf5 provides /usr/bin/tito``
    | Outputs which package provides the file ``/usr/bin/tito``

``dnf5 provides zless``
    | There is no ``Provides`` for zless but a package that contains ``zless`` exist.
    | Outputs the package that provides the file ``/usr/bin/zless``

.. note::
   New behavior: if one or more packages are not found by provides DNF5 will now return
   exit code 1 and list all provides that do not match at the end of the output.


``dnf5 provides rpm nonexistent_package``
    | Output provides for ``rpm`` and inform in ``stderr`` that ``nonexistent_package``
    | did not match any result.

See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
