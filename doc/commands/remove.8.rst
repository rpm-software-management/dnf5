.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _remove_command_ref-label:

###############
 Remove Command
###############

Synopsis
========

``dnf5 remove [options] <package-spec-NF>|@<group-spec>|@<environment-spec>...``


Description
===========

The ``remove`` command in ``DNF5`` is used for removing installed packages, groups or
environments from the system.

If you want to keep the dependencies that were installed together with the given package,
set the ``clean_requirements_on_remove`` configuration option to ``False``.


Options
=======

.. include:: ../_shared/options/installed-from-repo.rst

``--no-autoremove``
    | Disable removal of dependencies that are no longer used.

.. include:: ../_shared/options/transaction.rst


Examples
========

``dnf5 remove tito``
    | Remove the ``tito`` package.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
