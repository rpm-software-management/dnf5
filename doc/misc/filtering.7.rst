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

.. _filtering_misc_ref-label:

###################
 Packages Filtering
###################

Description
===========

Package filtering removes packages from the available package set, making them invisible to most ``DNF5`` commands. As a result, these packages cannot be included in any transaction.

There are several ways a package can be filtered out:

Global excludes filtering
=========================

Exclude filtering can be adjusted using the ``includepkgs`` or ``excludepkgs`` configuration options in the DNF5 configuration file. To disable excludes, you can use the ``disable_excludes`` configuration option:

``dnf5 --setopt=disable_excludes=* install bash``

For details about the configuration options see :manpage:`dnf5.conf(5)`, :ref:`DNF5 configuration reference <dnf5_conf-label>`.


Repository excludes filtering
=============================

Similar to global excludes, but this configuration is repository-specific and only affects packages within the repository where the excludes are set.


User excludes filtering
=======================

API users have an additional option how to fine-tune excluded packages using ``*_user_excludes()`` and ``*_user_includes`` methods of the ``PackageSack`` object. See `libdnf5::rpm::PackageSack`_.


Versionlock
===========

Additionally, the versionlock functionality is implemented using excludes filtering. However, these excludes are applied only during transactional operations.
For details see :manpage:`dnf5-versionlock(8)`, :ref:`Versionlock command <versionlock_command_ref-label>`.


Modular filtering
=================

For details on how modular filtering works please see :manpage:`dnf5-modularity(7)`, :ref:`the modularity documentation <modularity_misc_ref-label>`.

With modularity, only RPM packages from ``active`` module streams are included in the available package set. RPM packages from ``inactive`` module streams, as well as non-modular packages with the same name or provides as a package from an ``active`` module stream, are filtered out. Modular filtering is not applied to packages added from the command line, installed packages, or packages from repositories with ``module_hotfixes=true`` in their ``.repo`` file.

Disabling of modular filtering is not recommended, because it could cause the system to get into a broken state. To disable modular filtering for a particular repository, specify
``module_hotfixes=true`` in the ``.repo`` file or use ``--setopt=<repo_id>.module_hotfixes=true``.

..
    # TODO(mblaha) - `dnf5 module provides` command is not implemented yet
    # To discover the module which contains an excluded package use ``dnf5 module provides``.
