.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _downgrade_command_ref-label:

##################
 Downgrade Command
##################

Synopsis
========

``dnf5 downgrade [options] <package-spec-NPFB>...``


Description
===========

The ``downgrade`` command in ``DNF5`` is used to downgrade each package specified in ``package-spec-NPFB`` list to the
highest installable version of all known lower versions if possible. When the version is explicitly given
in the argument and it is lower than the version of the installed package then it downgrades to this one.


Options
=======

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.

``--skip-unavailable``
    | Allow skipping packages that are not possible to downgrade. All remaining packages will be downgraded.

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

``dnf5 downgrade nano-0:6.0-2.fc36``
    | Downgrade the ``nano`` package to the given version.

``dnf5 downgrade gcc glibc --allowerasing``
    | Downgrade ``gcc``, ``glibc`` packages and allow removing of installed packages when needed.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
