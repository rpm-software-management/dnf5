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

.. _remove_command_ref-label:

###############
 Remove Command
###############

Synopsis
========

``dnf5 remove [options] <package-spec-NF>|@<group-spec>|@<environment-spec>...``

``dnf5 remove --oldinstallonly [--limit=<LIMIT>] [<package-spec-NF>...]``


Description
===========

The ``remove`` command in ``DNF5`` is used for removing installed packages, groups or
environments from the system.

If you want to keep the dependencies that were installed together with the given package,
set the ``clean_requirements_on_remove`` configuration option to ``False``.

When the ``--oldinstallonly`` option is used, the command removes old installonly packages
(e.g. kernels) that exceed the configured ``installonly_limit``. The currently running kernel
is never removed. If package specs are provided, only matching installonly packages are
considered for removal.


Options
=======

.. include:: ../_shared/options/installed-from-repo.rst

``--no-autoremove``
    | Disable removal of dependencies that are no longer used.

``--oldinstallonly``
    | Remove old installonly packages that exceed the ``installonly_limit`` configuration value.
    | When used without package specs, all installonly package types are considered.
    | The currently running kernel is automatically skipped.

``--limit=<LIMIT>``
    | Override the ``installonly_limit`` configuration value. Must be greater than or equal to 1
      to keep at least the newest installed version. Can only be used with ``--oldinstallonly``.

.. include:: ../_shared/options/transaction.rst


Examples
========

``dnf5 remove tito``
    | Remove the ``tito`` package.

``dnf5 remove --oldinstallonly``
    | Remove all old installonly packages exceeding the configured ``installonly_limit``.

``dnf5 remove --oldinstallonly kernel``
    | Remove old kernel packages exceeding the configured ``installonly_limit``.

``dnf5 remove --oldinstallonly --limit=2``
    | Remove old installonly packages, keeping only the 2 newest versions of each.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
