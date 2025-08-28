.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

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
Both ``<remove-spec>`` and ``<install-spec>`` are ``<package-spec-NPFB>``


Options
=======

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

.. include:: ../_shared/options/installed-from-repo.rst

.. include:: ../_shared/options/from-repo.rst

.. include:: ../_shared/options/transaction.rst


Examples
========

``dnf5 swap mlocate plocate``
    | Remove the ``mlocate`` package and install the ``plocate`` instead in the single transaction.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
