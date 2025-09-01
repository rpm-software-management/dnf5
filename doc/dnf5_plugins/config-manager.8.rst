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

.. _config_manager_plugin_ref-label:

#######################
 Config-manager Command
#######################

Synopsis
========

``dnf5 config-manager <subcommand> [options] [arguments]``


Description
===========

Manages main configuration, repositories configuration, and variables.


Subcommands
===========

``addrepo --from-repofile=REPO_CONFIGURATION_FILE_URL [--create-missing-dir] [--overwrite] [--save-filename=FILENAME]``
    Adds a new repositories configuration file specified by URL or local path.

    The file is copied without changes. The destination file name can be defined using the ``--save-filename`` option,
    otherwise it is taken from the source specification. If the ``.repo`` extension is missing from the destination
    filename, it will be added.

    The destination directory path is the first path stored in the ``reposdir`` option (by default ``/etc/yum.repos.d``).
    Overwriting/replacing an existing file can be enabled using the ``--overwrite`` option.
    Before the new repositories configuration file is finally saved, it is analyzed and tested for validity.
    Repository IDs are also tested. Repositories with the same ID must not be defined in other configuration files.

``addrepo [--id=REPO_ID] <--set=REPO_OPTION=VALUE>+ [--add-or-replace] [--create-missing-dir] [--overwrite] [--save-filename=FILENAME]``
    Adds a new repository defined using user options.

    At least one of ``--set=baseurl=<URL>``, ``--set=mirrorlist=<URL>``, ``--set=metalink=<URL>`` must be set
    to a non-empty URL. The repository ID can be defined using the ``--id`` option, otherwise it is generated from the URL.
    The destination file name can be defined using the ``--save-filename`` option, otherwise it is set to the repository ID.
    If the ``.repo`` extension is missing from the destination filename, it will be added.

    The destination directory path is the first path stored in the ``reposdir`` option (by default ``/etc/yum.repos.d``).
    Overwriting/replacing an existing file can be enabled using the ``--overwrite`` option.
    Modifying an existing file can be enabled using the ``--add-or-replace`` option. Modification means
    that the new repository is added to the existing file or a repository with the same ID is replaced in the file.
    Before the new repository configuration is finally saved, the repository ID is tested. A repository
    with the same ID must not be defined in another configuration file.

``setopt [--create-missing-dir] <[repoid.]option_name=value>+``
    Sets options in the main configuration file and in the repositories configuration override file.

    The main configuration is read first from the files in the drop-in directories and then from the main configuration
    file (by default ``/etc/dnf/dnf.conf``). So, options from main configuration file are read last and override
    the values of the same options defined in the drop-in directories.

    The repositories configuration is read from the repository configuration files and then adjusted using repository
    configuration overrides. Repository configuration override files are read from the distribution repository override
    directory (``/usr/share/dnf5/repos.override.d``) and from the system repository override directory
    (``/etc/dnf/repos.override.d``). If a file with the same name is present in both override directories,
    only the file from the system override directory is used. Thus, the distribution override file can simply be masked
    by creating a file with the same name in the system override directory. All used override files are sorted
    alphabetically by their names and then applied in that order. The override from the next file overwrites
    the previous one---the last override wins.

    The ``setopt`` command writes the repositories configuration overrides to a file named ``99-config_manager.repo``
    located in the system repository override directory. Repository settings are written only to this override file.
    The original repository configuration file is not changed. Reposiory ID may contain globs.
    Override files also support globs. But the ``setopt`` command resolves the ``repoid`` pattern, and overrides are set
    for each matching repository independently. This means repositories added later will not be affected by these overrides.

``unsetopt <[repoid.]option_name>+``
    Removes options from the main configuration file and from the repositories configuration override file.

    The ``unsetopt`` command removes options from the main configuration file (by default ``/etc/dnf/dnf.conf``).
    However, the options may be still defined in configuration files in drop-in directories (for example,
    the default distribution configuration).

    The ``unsetopt`` command removes repository configuration overrides from the file named ``99-config_manager.repo``
    located in the system repository override directory. However, the overrides may be still defined in other repository
    override files (for example, the default distribution overrides). Empty sections are removed from the configuration
    override file. Repository ID may contain globs. In this case, the ``repoid`` pattern is resolved and the override
    is removed from all matching sections.

``setvar [--create-missing-dir] <variable_name=value>+``
    Sets one or more variables.

    Variables are loaded from multiple directories. The list of directory paths is taken from the ``varsdir`` option.
    The ``setvar`` command stores variables to the last directory in the list (by default ``/etc/dnf/vars``).
    Variables from this directory are read last and override the values of the same variables defined in previous
    directories. In other words, the last directory has the highest priority.

    Note:
    The variables ``releasever_major`` and ``releasever_minor`` are read-only. Their values are generated from the ``releasever`` variable.

``unsetvar <variable>+``
    Removes variables.

    Variables are loaded from multiple directories. The list of directory paths is taken from the ``varsdir`` option.
    The ``unsetvar`` command removes variables from the last directory in the list (by default ``/etc/dnf/vars``).
    So, the variable may still exist in another directory in the list (for example, the default distribution value).

    Note:
    The variables ``releasever_major`` and ``releasever_minor`` are generated automatically and cannot be removed.


.. note::
   Override directories are also listed with examples in :ref:`Drop-in repo directories<drop_in_repo_directories-label>`.

Options
=======

``--add-or-replace``
    Allow adding or replacing the repository with the same ID in the existing configuration file.

``--create-missing-dir``
    Allow creation of missing directories.

``--from-repofile=REPO_CONFIGURATION_FILE_URL``
    Specifies the source configuration file with the new repositories.

``--id=REPO_ID``
    Set ID for newly created repository.

``--overwrite``
    Allow replacing the existing repository configuration file by new one.

``--save-filename=FILENAME``
    Set the name of the new repository configuration file. The ``.repo`` extension is added if it is missing.

``--set=REPO_OPTION=VALUE``
    Set option in newly created repository.


Examples
========

``dnf5 config-manager addrepo --from-repofile=http://example.com/some/additional.repo``
    Download ``additional.repo``, test it, and put it in repository configuration directory.

``dnf5 config-manager addrepo --set=baseurl=http://example.com/different/repo``
    Create new repo file with ``http://example.com/different/repo`` as ``baseurl`` and enable it. The repository ID and target file name is generated from ``baseurl``.

``dnf5 config-manager addrepo --set=baseurl=http://example.com/different/rep --id=example --set=enabled=0``
    Create new repo file with ``http://example.com/different/repo`` as ``baseurl``. Set repository ID to ``example`` and disable it.

``dnf5 config-manager setopt repoid1.enabled=1 repoid2.enabled=0``
    Sets override to enable repository identified by ``repoid1`` and disable repository identified by ``repoid2``.

``dnf5 config-manager setopt repo1.proxy=http://proxy.example.com:3128/ repo2.proxy=http://proxy.example.com:3128/``
    Sets override for ``proxy`` option in repositories with repository IDs ``repo1`` and ``repo2``.

``dnf5 config-manager setopt '*-debuginfo.pkg_gpgcheck=0'``
    Sets override for the ``pkg_gpgcheck`` option in all repositories whose repository ID ends with ``-debuginfo``.

``dnf5 config-manager unsetopt '*-debuginfo.pkg_gpgcheck'``
    Remove override for the ``pkg_gpgcheck`` option in all repositories whose repository ID ends with ``-debuginfo``.

``dnf5 config-manager setopt keepcache=1 log_size=10M``
    Enables the ``keepcache`` main option and sets the maximum size of logger files to 10 mebibytes (10 * 1024 * 1024 bytes).

``dnf5 config-manager unsetopt keepcache log_size``
    Removes ``keepcache`` and ``log_size`` from the main configuration file.

``dnf5 config-manager setvar --create-missing-dir myvar1=value1 myvar2=value2``
    Sets the variables ``myvar1`` and ``myvar2``. Directory for the variables is created if it does not exist.

``dnf5 config-manager unsetvar myvar1 myvar2``
    Removes ``myvar1`` and ``myvar2`` variables.

See Also
========

Configuration:
    | :manpage:`dnf5.conf(5)`, :ref:`DNF5 Configuration Reference <dnf5_conf-label>`
