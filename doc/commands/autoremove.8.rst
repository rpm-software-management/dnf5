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

.. _autoremove_command_ref-label:

###################
 Autoremove Command
###################

Synopsis
========

``dnf5 autoremove``


Description
===========

The ``autoremove`` command in ``DNF5`` is used for removing unneeded packages
from the system.

Unneeded packages are all "leaf" packages that were originally installed as
dependencies of user-installed packages, but which are no longer required by
any such package.

Installonly packages (e.g. kernels) are never automatically removed by this
command even if they were installed as dependencies.


Options
=======

.. include:: ../_shared/options/transaction.rst
