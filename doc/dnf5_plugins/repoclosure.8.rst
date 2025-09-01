.. Copyright Contributors to the DNF5 project.
..
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

.. _repoclosure_plugin_ref-label:

####################
 Repoclosure Command
####################

Synopsis
========

``dnf5 repoclosure [options] [<pkg-spec>...]``


Description
===========

The ``repoclosure`` command allows you to analyze package metadata from multiple repositories. It checks all dependencies of the packages and provides a list of packages that have unresolved dependencies.

By default, ``repoclosure`` considers all enabled repositories when checking dependencies. However, you can customize the set of repositories by using standard DNF5 options such as ``--repo``, ``--enable-repo``, or ``--disable-repo``.


Options
=======

``--arch <arch>``
    | Query only packages for specified architecture, can be specified multiple times (default is all compatible architectures with your system).

``--best``
    | Check only the newest packages per arch.

``--check=REPO_ID,...``
    | Specify repositories to check, can be specified multiple times (default is all enabled repositories).
    | Accepted values are repository ids, or a glob of ids.

``--newest``
    | Check only the newest packages in the repos.

``<pkg-spec>``
    | Check closure for this package only.


Examples
========

``dnf5 repoclosure``
    | Display a list of unresolved dependencies for all enabled repositories.

``dnf5 repoclosure --repo rawhide --arch noarch --arch x86_64``
    | Display a list of unresolved dependencies for rawhide repository and packages with architecture noarch and x86_64.

``dnf5 repoclosure --repo rawhide zmap``
    | Display a list of unresolved dependencies for zmap package from rawhide repository.

``dnf5 repoclosure --repo rawhide --check myrepo``
    | Display a list of unresolved dependencies for myrepo, an add-on for the rawhide repository.
