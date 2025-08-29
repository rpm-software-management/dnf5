.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _reinstall_command_ref-label:

##################
 Reinstall Command
##################

Synopsis
========

``dnf5 reinstall [global options] <package-spec-NPFB>...``


Description
===========

The ``reinstall`` command in ``DNF5`` is used for reinstalling packages defined in
``package-spec-NPFB`` arguments.


Options
=======

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.

``--skip-unavailable``
    | Allow skipping packages that are not possible to reinstall. All remaining packages will be reinstalled.

``--allow-downgrade``
    | Enable downgrade of dependencies when resolving the requested operation.

``--no-allow-downgrade``
    | Disable downgrade of dependencies when resolving the requested operation.

.. include:: ../_shared/options/installed-from-repo.rst

.. include:: ../_shared/options/from-repo.rst

``--downloadonly``
    | Download the resolved package set without executing an RPM transaction.

.. include:: ../_shared/options/transaction.rst


Examples
========

``dnf5 reinstall tito``
    | Reinstall the ``tito`` package.

``dnf5 reinstall ~/Downloads/tito-0.6.21-1.fc36.noarch.rpm``
    | Reinstall the ``tito`` package from the local rpm file.
    | This can be useful f.e. when the given package is not available in any enabled repository.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
