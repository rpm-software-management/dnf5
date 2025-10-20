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

.. _modularity_misc_ref-label:

####################
 Modularity Overview
####################

Description
===========

Modularity is an alternative way of building, organizing and delivering packages.
For more details see: https://docs.pagure.org/modularity/.


Definitions
===========

modulemd
    Every repository can contain ``modules`` metadata with modulemd documents.
    These documents hold metadata about modules such as ``Name``, ``Stream`` or list of packages.

(non-modular) package
    Package that doesn't belong to a module.

modular package
    Package that belongs to a module. It is listed in modulemd under the ``artifacts`` section.
    Modular packages can be also identified by having ``%{modularitylabel}`` RPM header set.

(module) stream
    Stream is a collection of packages, a virtual repository. It is identified with
    ``Name`` and ``Stream`` from modulemd separated with colon, for example "postgresql:9.6".

    Module streams can be ``active`` or ``inactive``. ``active`` means the RPM
    packages from this stream are included in the set of available packages.
    Packages from ``inactive`` streams are filtered out.  Streams are
    ``active`` either if marked as ``default`` or if they are explicitly
    ``enabled`` by a user action. Streams that satisfy dependencies of
    ``default`` or ``enabled`` streams are also considered ``active``.  Only
    one stream of a particular module can be ``active`` at a given point in
    time.


Package filtering
=================

Without modules, packages with the highest version are used by default.

Module streams can distribute packages with lower versions than available in the
repositories available to the operating system. To make such packages available
for installs and upgrades, the non-modular packages are filtered out when their
name or provide matches against a modular package name from any enabled, default,
or dependent stream. Modular source packages will not cause non-modular binary
packages to be filtered out.


Demodularized rpms
==================

Contains names of RPMs excluded from package filtering for particular module stream. When defined in the latest active
module, non-modular RPMs with the same name or provide which were previously filtered out will reappear.


Hotfix repositories
===================

In special cases, a user wants to cherry-pick individual packages provided outside module
streams and make them available on along with packages from the active streams.
Under normal circumstances, such packages are filtered out or rejected from getting on the system by
Fail-safe mechanisms.
To make the system use packages from a repository regardless of their modularity,
specify ``module_hotfixes=true`` in the .repo file. This protects the repository from package filtering.

Please note the hotfix packages do not override module packages, they only become
part of available package set. It's the package ``Epoch``, ``Version`` and ``Release``
what determines if the package is the latest.


Fail-safe mechanisms
====================


Repositories with module metadata are unavailable
=================================================

When a repository with module metadata is unavailable, package filtering must keep working.
Non-modular RPMs must remain unavailable and must never get on the system.

This happens when:

* user disables a repository via ``--disable-repo`` or uses ``--repo``
* user removes a .repo file from disk
* repository is not available and has ``skip_if_unavailable=true``

DNF5 keeps copies of the latest modulemd for every active stream
and uses them if there's no modulemd available for the stream.
This keeps package filtering working correctly.

The copies are made any time a transaction is resolved and started.
That includes RPM transactions as well as any ``dnf5 module <enable|disable|reset>`` operations.

When the fail-safe data is used, DNF5 show such modules as part of @modulefailsafe repository.


Orphaned modular packages
=========================

All packages that are built as a part of a module have ``%{modularitylabel}`` RPM header set.
If such package becomes part of RPM transaction and cannot be associated with any available
modulemd, DNF5 prevents from getting it on the system (package is available, but cannot be
installed, upgraded, etc.). Packages from Hotfix repositories or Commandline repository are not
affected by Fail-safe mechanisms.
