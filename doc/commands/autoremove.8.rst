.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

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
