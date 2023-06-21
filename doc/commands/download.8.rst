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

.. _download_command_ref-label:

#################
 Download Command
#################

Synopsis
========

``dnf5 download [options] <package-spec>...``


Description
===========

The ``download`` command in ``DNF5`` is used for downloading binary and source packages
defined in ``package-spec`` arguments to the current working directory.


Options
=======

``--resolve``
    | Resolve dependencies of specified packages and download missing ones.

``--alldeps``
    | To be used together with ``--resolve``, it downloads all dependencies, not skipping the already installed ones.

``--destdir=<path>``
    Set directory used for downloading packages to. Default location is to the current working directory.

``--forcearch=<arch>``
    | Force the use of a specific architecture.
    | :ref:`See <forcearch_misc_ref-label>` :manpage:`dnf5-forcearch(7)` for more info.


Examples
========

``dnf5 download kernel-headers-0:5.17.0-300.fc36.i686``
    | Download the ``kernel-headers`` package using the full NEVRA format.

``dnf5 download rpm rpm-devel``
    | Download all packages having the name of ``rpm`` or ``rpm-devel``.

``dnf5 download maven-compiler-plugin --resolve --alldeps``
    | Download the ``maven-compiler-plugin`` package with all its dependencies.

``dnf5 download --destdir /tmp/my_packages maven-compiler-plugin``
    | Download the ``maven-compiler-plugin`` package to ``/tmp/my_packages`` directory.

See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
