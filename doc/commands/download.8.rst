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

.. _download_command_ref-label:

#################
 Download Command
#################

Synopsis
========

``dnf5 download [options] <package-spec-NPFB>...``


Description
===========

The ``download`` command in ``DNF5`` is used for downloading binary and source packages
defined in ``package-spec-NPFB`` arguments to the current working directory.


Options
=======

``--arch``
    | Limit to packages of given architectures. This option can be used multiple times.

``--resolve``
    | Resolve dependencies of specified packages and download missing ones.

``--alldeps``
    | To be used together with ``--resolve``, it downloads all dependencies, not skipping the already installed ones.

.. include:: ../_shared/options/from-repo.rst

.. include:: ../_shared/options/from-vendor.rst

``--destdir=<path>``
    | Set directory used for downloading packages to. Default location is to the current working directory.

``--skip-unavailable``
    | Allow skipping packages that are not available in repositories. All available packages will be downloaded.

``--srpm``
    | Download the source rpm. Enables source repositories of all enabled binary repositories.

``--debuginfo``
    | Download the debuginfo rpm. Enables debuginfo repositories of all enabled binary repositories.

``--url``
    | Prints the list of URLs where the rpms can be downloaded instead of downloading.

``--urlprotocol``
    | To be used together with ``--url``. It filters out the URLs to the specified protocols: ``http``, ``https``, ``ftp``, or ``file``. This option can be used multiple times.

``--allmirrors``
    | To be used together with ``--url``. It prints out space-separated URLs from all available mirrors for each package.


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

``dnf5 download --url --urlprotocol http python``
    | List the http URL to download the python package.

``dnf5 download python --arch x86_64``
    | Downloads python with the ``x86_64`` architecture.

``dnf5 download dnf5 --srpm``
    | Download the ``dnf5`` source rpm.

``dnf5 download kernel --debuginfo``
    | Download the ``kernel`` debuginfo rpm.

See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
