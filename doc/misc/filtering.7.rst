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

..
    # TODO(jkolarik): review, fix according to DNF5 state

.. _filtering_misc_ref-label:

###################
 Packages Filtering
###################

Description
===========

Package filtering filters packages out from the available package set, making them invisible to most
of the ``DNF5`` commands. They cannot be used in a transaction. Packages can be filtered out by either
`Exclude Filtering` or `Modular Filtering`.


Exclude Filtering
=================

Exclude Filtering is a mechanism used by a user or by the ``DNF5`` plugin to modify the set of available
packages. Exclude Filtering can be modified by either ``includepkgs`` or ``excludepkgs`` configuration options in
configuration files. In addition to user-configured excludes, plugins can also extend the set of excluded packages. 
To disable excludes from the ``DNF5`` plugin you can use the ``--disable-plugin`` command line option.

To disable all excludes for e.g. the install command you can use the following combination
of command line options:

``dnf5 --disableexcludes=all --disable-plugin="*" install bash``


Modular Filtering
=================

Please see `the modularity documentation` for details on how Modular Filtering works.

With modularity, only RPM packages from ``active`` module streams are included in the available
package set. RPM packages from ``inactive`` module streams, as well as non-modular packages with
the same name or provides as a package from an ``active`` module stream, are filtered out. Modular
filtering is not applied to packages added from the command line, installed packages, or packages
from repositories with ``module_hotfixes=true`` in their ``.repo`` file.

Disabling of modular filtering is not recommended, because it could cause the system to get into
a broken state. To disable modular filtering for a particular repository, specify
``module_hotfixes=true`` in the ``.repo`` file or use ``--setopt=<repo_id>.module_hotfixes=true``.

To discover the module which contains an excluded package use ``dnf5 module provides``.


Examples
========

