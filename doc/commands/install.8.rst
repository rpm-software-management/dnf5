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

.. _install_command_ref-label:

################
 Install Command
################

Synopsis
========

``dnf5 install [options] <package-spec-NPFB>|@<group-spec>|@<environment-spec>...``


Description
===========

The ``install`` command in ``DNF5`` is used for installing packages, groups or environments.

When installing packages defined in ``package-spec-NPFB`` arguments, ``DNF5`` ensures that the packages
and their dependencies are installed on the system.
If the specified packages are already installed, DNF5 does not check their dependencies again and
simply verifies that the packages themselves are present.

When ``package-spec-NPFB`` to specify the exact version of the package is given, DNF will install the
desired version, no matter which version of the package is already installed. The former version of
the package will be removed in the case of non-installonly package.

On the other hand if ``package-spec-NPFB`` specifies only a name and obsoletes are enabled, DNF also
takes into account packages obsoleting that name.
Note that this can lead to seemingly unexpected results if the version of an installed package is obsoleted
and the installed package also has newer version available. It creates a split in upgrade-path and both ways
are considered correct, the resulting package is picked simply by lexicographical order.

When installing groups defined in ``group-spec`` arguments, ``DNF5`` ensures that the groups and
their packages are installed on the system. Installs only group packages matching configured package
type. See :manpage:`dnf5.conf(5)`, :ref:`group_package_types <group_package_types_options-label>`.

When installing environments defined in ``environment-spec`` arguments, ``DNF5`` ensures that the
environments and their groups are installed on the system.


Options
=======

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

.. include:: ../_shared/options/from-repo.rst

``--downloadonly``
    | Download the resolved package set without executing an RPM transaction.

.. include:: ../_shared/options/transaction.rst

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

``dnf5 install tito``
    | Install the ``tito`` package.

``dnf5 install ~/Downloads/tito-0.6.21-1.fc36.noarch.rpm``
    | Install the local rpm file from the given location.

``dnf5 install tito-0.6.21-1.fc36``
    | Install the ``tito`` package in defined version.
    | If the package is already installed, it will automatically try to downgrade or upgrade to the given version.

``dnf5 install --advisory=FEDORA-2022-07aa56297a \*``
    | Install all the packages that belong to the ``FEDORA-2022-07aa56297a`` advisory.


See Also
========

    | :manpage:`dnf5-advisory(8)`, :ref:`Advisory command <advisory_command_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
