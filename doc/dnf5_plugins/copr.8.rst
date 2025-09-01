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

.. _copr_plugin_ref-label:

#############
 Copr Command
#############

Synopsis
========

``dnf5 copr <subcommand> [options] [arguments]``


Description
===========

The ``copr`` command in ``DNF5`` is used to manage Copr repositories (add-ons provided by users/community/third-party) on the local system.


Subcommands
===========

``list``
    | List Copr repositories.

``enable <project-spec> [<chroot>]``
    | Download the repository info from a Copr server and install it as a `/etc/yum.repos.d/*.repo` file.

``disable <project-spec>``
    | Disable specified Copr repository (if exists), keep `/etc/yum.repos.d/*.repo` file - just set enabled=0.

``remove <project-spec>``
    | Remove specified Copr repository from the system. Also removes the `/etc/yum.repos.d/*.repo` file.

``debug``
    | Print info about the system useful for debugging.


Options
=======

``--hub``
    | Copr hub (the web-UI/API server) hostname. Defaults to `copr.fedorainfracloud.org`.


Arguments
=========

``<project-spec>``
    Copr project ID to enable. Use either a format OWNER/PROJECT
    or HUB/OWNER/PROJECT.

    HUB can be either the Copr frontend hostname (e.g. `copr.fedorainfracloud.org`)
    or the shortcut (e.g. `fedora`).
    If HUB is not specified, the default one, or `--hub <ARG>`, is used.

    OWNER is either a username, or a @groupname.

    PROJECT can be a simple project name, or a "project directory" containing colons,
    e.g. `project:custom:123`.

    Example: `fedora/@footeam/coolproject`.

``<chroot>``
    Chroot specified in the NAME-RELEASE-ARCH format, e.g. `fedora-rawhide-ppc64le`.
    When not specified, the `dnf copr` command attempts to detect it.


Examples
========

``dnf5 copr enable rhscl/perl516 epel-6-x86_64``
    | Enable the rhscl/perl516 Copr repository, using the epel-6-x86_64 chroot.

``dnf5 copr disable rhscl/perl516``
    | Disable the rhscl/perl516 Copr repository

``dnf5 copr list``
    | List Copr repositories configured on the system.
