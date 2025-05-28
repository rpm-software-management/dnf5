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

.. _swap_command_ref-label:

#############
 Swap Command
#############

Synopsis
========

``dnf5 swap [options] <remove-spec> <install-spec>``


Description
===========

The ``swap`` command in ``DNF5`` is used for removing a package while installing
a different one in a single transaction.


Options
=======

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

.. include:: ../_shared/options/from-repo.rst

``--offline``
    | Store the transaction to be performed offline. See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`.


Examples
========

``dnf5 swap mlocate plocate``
    | Remove the ``mlocate`` package and install the ``plocate`` instead in the single transaction.
