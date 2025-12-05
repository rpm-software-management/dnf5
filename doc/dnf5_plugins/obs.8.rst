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

.. _obs_plugin_ref-label:

#############
 OBS Command
#############

Synopsis
========

``dnf5 obs <subcommand> [options] [arguments]``


Description
===========

The ``obs`` command in ``DNF5`` is used to manage OBS repositories (add-ons provided by users/community/third-party) on the local system.


Subcommands
===========

``list``
    | List OBS repositories.

``enable <project-spec> [<chroot>]``
    | Download the repository info from an OBS server and install it as a `/etc/yum.repos.d/*.repo` file.

``disable <project-spec>``
    | Disable specified OBS repository (if exists), keep `/etc/yum.repos.d/*.repo` file - just set enabled=0.

``remove <project-spec>``
    | Remove specified OBS repository from the system. Also removes the `/etc/yum.repos.d/*.repo` file.

``debug``
    | Print info about the system useful for debugging.


Options
=======

``--hub``
    | OBS hub (the web-UI/API server) hostname. Defaults to `build.opensuse.org`.


Arguments
=========

``<project-spec>``
    OBS project ID to enable. Use either a format PROJECT/REPONAME
    or HUB/PROJECT/REPONAME.

    HUB can be either the OBS frontend hostname (e.g. `build.opensuse.org`)
    or the shortcut (e.g. `opensuse`).
    If HUB is not specified, the default one, or `--hub <ARG>`, is used.

    PROJECT is a "project directory" containing colons, e.g. `home:user:subproject`.

    REPONAME is the repository name in the project, e.g. `openSUSE_Tumbleweed`.

    Example: `opensuse/home:user:subproject/openSUSE_Tumbleweed`.


Examples
========

``dnf5 obs enable home:user/sles15``
    | Enable the `sles15` repository belonging to the `home:user` project.

``dnf5 obs disable home:user:mysubproject/fedora43``
    | Disable the `fedora43` repository belonging to the `home:user:mysubproject` project.

``dnf5 obs list``
    | List OBS repositories configured on the system.
