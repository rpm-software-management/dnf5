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

.. _forcearch_misc_ref-label:

####################
 Forcearch Parameter
####################

Description
===========

The ``--forcearch=<arch>`` parameter overrides the system architecture detected by DNF 5. It allows querying repositories for packages not compatible with your host system and installing them. Any architecture can be specified, but using a package with an architecture not supported natively by your CPU will require emulation of some kind, e.g. using qemu-user-static.

``--forcearch`` is supported by the following commands: ``distro-sync``, ``download``, ``group``,  ``info``, ``install``, ``list``, ``makecache``, ``repo``, ``repoquery``, ``search``, and ``swap``.


Examples
========

``dnf5 install --forcearch=aarch64 my-example-package``
    Installs the version of ``my-example-package`` for the AArch64 architecture, regardless of the architecture of the host system.

``dnf5 download --forcearch=s390x hello``
    Downloads the ``hello`` package for the s390x architecture.

``dnf5 repoquery --forcearch=aarch64 --arch=aarch64``
    Query all packages available for the AArch64 architecture. If your system has a different native architecture, then both ``--arch`` and ``--forcearch`` are necessary here. ``--arch`` will filter for only packages with the ``aarch64`` architecture, and ``--forcearch`` sets the "arch" and "basearch" substitution variables to ensure the correct repositories are queried.


See Also
========

    | :ref:`Tutorial to override the system architecture for C++ API users <tutorial-session-force-arch-label>`
    | :ref:`Tutorial to override the system architecture for Python API users <tutorial-bindings-python3-session-force-arch-label>`
