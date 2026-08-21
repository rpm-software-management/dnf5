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

Configuration for the ``obs`` command is located in the ``/etc/dnf/plugins/obs.conf`` file, as well as drop-in files (with a ``.conf`` extension) in the ``/etc/dnf/plugins/obs.d/`` directory.

The configuration files are INI-format and may contain multiple sections, with each section containing key-value pairs.

The obs plugin first performs hostname lookup by looking for a section with the same name as the provided hubspec, and if found, using the value of its ``hostname`` key. All additional configuration values (except ``download_hostname``) are looked up using the section matching the hostname, or the default section (named ``DEFAULT``).

The obs plugin attempts to download the repo file directly from the configured ``download_hostname`` server using the configured ``download_protocol``, ``download_port``, and ``download_url_prefix``. If this fails, it will fall back to downloading the HTML repository state web page from the Web-UI server, using the ``hostname``, ``protocol``, ``port``, and ``url_prefix`` from the hostname section, and parsing that HTML to determine the download URL to use for the repo file.

The obs configuration has built-in configuration to handle communication with the official OBS instance, as well as built-in default configuration to communicate with any unofficial OBS instance that is using the default configuration (which sets up the download server to use http on port 82).

To view the current configuration, see ``dnf obs debug``.


Subcommands
===========

``list``
    | List OBS repositories.

``enable <project-spec>``
    | Download the repository info from an OBS server and install it as a `/etc/yum.repos.d/*.repo` file.

``disable <project-spec>``
    | Disable specified OBS repository (if exists), keep `/etc/yum.repos.d/*.repo` file - just set enabled=0.

``remove <project-spec>``
    | Remove specified OBS repository from the system. Also removes the `/etc/yum.repos.d/*.repo` file.

``debug``
    | Print info about the system useful for debugging, including the current obs configuration (built-in as well as read from configuration files).


Options
=======

``--hub``
    | OBS hub (the web-UI server) hostname. Defaults to `build.opensuse.org`.


Arguments
=========

``<project-spec>``
    OBS project ID to enable. Use either a format PROJECT/REPONAME
    or HUB/PROJECT/REPONAME.

    HUB can be either the OBS frontend hostname (e.g. `build.opensuse.org`)
    or the shortcut (e.g. `opensuse`). If HUB is not specified, the default
    one, or `--hub <ARG>`, is used.

    PROJECT is a "project directory" containing colons, e.g. `home:user:subproject`.

    REPONAME is the repository name in the project, e.g. `openSUSE_Tumbleweed`.

    Example: `opensuse/home:user:subproject/openSUSE_Tumbleweed`.


Examples
========

``dnf5 obs enable home:user/sles15``
    | Enable the `sles15` repository belonging to the `home:user` project.

``dnf5 obs disable home:user:mysubproject/Fedora_43``
    | Disable the `Fedora_43` repository belonging to the `home:user:mysubproject` project.

``dnf5 obs list``
    | List OBS repositories configured on the system.
