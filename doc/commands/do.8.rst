..
    Copyright Contributors to the DNF5 project.

    This file is part of DNF5: https://github.com/rpm-software-management/dnf5/

    DNF5 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    DNF5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DNF5.  If not, see <https://www.gnu.org/licenses/>.

.. _do_command_ref-label:

################
 Do Command
################

Synopsis
========

``dnf5 do [options] [arguments]``


Description
===========

The ``do`` command is a universal command for package management in DNF5. It allows a user to define multiple
actions (``install``, ``remove``, ``upgrade``, ``downgrade``, and ``reinstall``).

Unlike a series of specific commands (``install``, ``remove``, ``upgrade``, ``downgrade``, and ``reinstall``),
the ``do`` command handles all requested actions together and performs them in a single transaction.
This simplifies complex operations and makes them more efficient.

Another difference is that the ``do`` command allows for the explicit specification of object types
in the transaction: ``package``, ``group``, or ``auto``. For the ``group`` type, it is possible to specify whether
a group ID or a group name is used. By default, the ``auto`` type is active. In this mode, the ``do`` command
behaves like other commands, inferring the object's type from its specification. For example, if a specification starts
with the ``@`` character, it is treated as a group ID or a module.

Individual actions and object types are specified by options and are combinable.

Other options have the same meaning and use as with the commands (``install``, ``remove``, ``upgrade``, ``downgrade``,
and ``reinstall``). The exceptions are the ``--installed-from-repo`` and ``--from-repo`` options. While they have
the same meaning, they only apply to the packages that follow them on the command line. Additionally, they can be
repeated with different values.


Options
=======

``--action``
    | Action to be done on the following items.

    This is a mandatory option that must precede the transaction's items (objects). The do command needs
    this information to know which action to perform on the specified items.
    Supports: ``install``, ``remove``, ``upgrade``, ``downgrade``, ``reinstall``.

``--type``
    | Type of the following items.

    Supports: ``auto``, ``package``, ``group``. ``auto`` is the default until the ``type`` option is used.

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.

``--skip-unavailable``
    | Allow skipping packages that are not available in repositories. All available packages will be installed.

``--allow-downgrade``
    | Enable downgrade of dependencies when resolving the requested operation.

``--no-allow-downgrade``
    | Disable downgrade of dependencies when resolving the requested operation.

.. include:: ../_shared/options/installed-from-repo.rst

.. include:: ../_shared/options/from-repo.rst

``--downloadonly``
    | Download the resolved package set without executing an RPM transaction.

``--offline``
    | Store the transaction to be performed offline. See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`.

.. include:: ../_shared/options/advisories.rst

.. include:: ../_shared/options/advisory-severities.rst

.. include:: ../_shared/options/bzs.rst

.. include:: ../_shared/options/cves.rst

.. include:: ../_shared/options/security.rst

.. include:: ../_shared/options/bugfix.rst

.. include:: ../_shared/options/enhancement.rst

.. include:: ../_shared/options/newpackage.rst


Examples
========

``dnf5 do --action=install tito``
    Install the ``tito`` package.

``dnf5 do --action=remove sddm-wayland-plasma --action=install sddm-x11``
    Replace package ``sddm-wayland-plasma`` with ``sddm-x11``.

``dnf5 do --action=install --type=group id=office 'name=Games and Entertainment' --type=package iftop --action=remove atop``
    Install the ``office`` group (by ID), the ``Games and Entertainment`` group (by name), and the ``iftop`` package,
    and remove the ``atop`` package.

``dnf5 do --action=install pkg --from-repo=myrepo1 pkg1 --from-repo=myrepo2 pkg2 --action=remove --installed-from-repo=compromised_repo '*'``
    Install the ``pkg`` package from any enabled repository, as well as package ``pkg1`` from the ``myrepo1``
    repository and package ``pkg2`` from the ``myrepo2`` repository. Dependencies for these packages are installed
    from any enabled repository. Additionally, remove all packages installed from the ``compromised_repo``.


See Also
========

    | :manpage:`dnf5-install(8)`, :ref:`Install command <install_command_ref-label>`
    | :manpage:`dnf5-remove(8)`, :ref:`Remove command <remove_command_ref-label>`
    | :manpage:`dnf5-upgrade(8)`, :ref:`Upgrade command <upgrade_command_ref-label>`
    | :manpage:`dnf5-downgrade(8)`, :ref:`Downgrade command <downgrade_command_ref-label>`
    | :manpage:`dnf5-reinstall(8)`, :ref:`Reinstall command <reinstall_command_ref-label>`
    | :manpage:`dnf5-advisory(8)`, :ref:`Advisory command <advisory_command_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
