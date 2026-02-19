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

.. _repo_command_ref-label:

#############
 Repo Command
#############

Synopsis
========

``dnf5 repo <subcommand> [options] [<repo-spec>...]``


Description
===========

The ``repo`` command in ``DNF5`` offers several queries for getting information
about repositories configured on the system.


Subcommands
===========

``list``
    | List available repositories.

``info``
    | Show detailed info about repositories.


Options
=======

``--all``
    | Show information about all known repositories.

``--enabled``
    | Show information only about enabled repositories.
    | This is the default behavior.

``--disabled``
    | Show information only about disabled repositories.


Examples
========

``dnf5 repo info --all``
    | Print detailed info about all known repositories.

``dnf5 repo list --disabled *-debuginfo``
    | Print disabled repositories related to debugging.

``dnf5 config-manager setopt repo_id.enabled=0``
    | Persistently disable repository using the config-manager plugin command.
    | See :manpage:`dnf5-config-manager(8)` for more details.

JSON Output
===========

* ``dnf5 repo list --json``

  The command returns a JSON array of objects, each describing one repository.
  Each object contains the following fields:

  - ``id`` (string) - Repository ID.
  - ``name`` (string) - Repository name.
  - ``is_enabled`` (boolean) - Repository status, either ``true`` (enabled),
    or ``false`` (disabled).

* ``dnf5 repo info --json``

  The command returns a JSON array of objects, each describing one repository.
  Each object contains the following fields:

  - ``id`` (string) - Repository ID.
  - ``name`` (string) - Repository name.
  - ``is_enabled`` (boolean) - Repository status, either ``true`` (enabled),
    or ``false`` (disabled).
  - ``priority`` (integer) - Repository priority.
  - ``cost`` (integer) - Repository cost.
  - ``type`` (string) - Repository type.
  - ``exclude_pkgs`` (array of strings) - List of excluded packages.
  - ``include_pkgs`` (array of strings) - List of included packages.
  - ``timestamp`` (integer) - Timestamp of the last metadata update,
    UNIX time.
  - ``metadata_expire`` (integer) - Metadata expiration time. If not set, value
    is taken from global config.
  - ``skip_if_unavailable`` (boolean) - Whether to skip the repository
    if it is unavailable.
  - ``repo_file_path`` (string) - Path to the repository file.
  - ``base_url`` (array of strings) - List of base URLs. They are “effective”
    base URLs, i.e., after expanding any variables included.
  - ``metalink`` (string) - Metalink URL.
  - ``mirrorlist`` (string) - Mirrorlist URL.
  - ``gpg_key`` (array of strings) - List of OpenPGP keys.
  - ``repo_gpgcheck`` (boolean) - Whether to perform GPG check
    of the repository metadata.
  - ``pkg_gpgcheck`` (boolean) - Whether to perform GPG check
    of the packages.
  - ``available_pkgs`` (integer) - Number of available packages
    in the repository.
  - ``pkgs`` (integer) - Number of packages
    in the repository.
  - ``size`` (integer) - Total size of packages
    in the repository, in bytes.
  - ``content_tags`` (array of strings) - List of content tags.
  - ``distro_tags`` (array of strings) - List of distro tags.
  - ``revision`` (string) - Repository revision.
  - ``max_timestamp`` (integer) - Maximum timestamp from repomd records;
    UNIX time.

For more details about the fields, see the ``REPO OPTIONS`` section in dnf5.conf(5).

See Also
========

      :manpage:`dnf5.conf(5)`, :ref:`Repo options <repo_options-label>`
