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

.. _command_ref-label:

################################
 DNF5 Package Management Utility
################################

..
    # TODO(jkolarik): unify first man page structure with the help output, especially the commands
                      grouping - think about it, some groups and related commands don't make much sense
    # TODO(jkolarik): add notes about mutually exclusive options
    # TODO(jkolarik): add crosslinks where possible
    # TODO(jkolarik): review against DNF4 if nothing important is missing
    # TODO(jkolarik): add misc page about advisories?

Synopsis
========

``dnf5 <command> [options] [<args>...]``


Description
===========

`DNF5`_ is the new version of `DNF`_, a package manager for RPM-based Linux distributions. It has been completely
rewritten in C++ aiming for better performance and reducing external dependencies.


Commands
========

Here is the list of the available commands.
For more details see the separate man page for the specific command, f.e. ``man dnf5 install``.

:ref:`advisory <advisory_command_ref-label>`
    | Manage advisories.

:ref:`autoremove <autoremove_command_ref-label>`
    | Remove unneeded packages.

:ref:`check <check_command_ref-label>`
    | Check for problems in the package database.

:ref:`check-upgrade <check-upgrade_command_ref-label>`
    | Check for available package upgrades.

:ref:`clean <clean_command_ref-label>`
    | Remove or invalidate cached data.

:ref:`distro-sync <distro-sync_command_ref-label>`
    | Upgrade or downgrade installed packages to the latest available version.

:ref:`downgrade <downgrade_command_ref-label>`
    | Downgrade packages.

:ref:`download <download_command_ref-label>`
    | Download packages.

:ref:`environment <environment_command_ref-label>`
    | Manage comps environments.

:ref:`group <group_command_ref-label>`
    | Manage comps groups.

:ref:`history <history_command_ref-label>`
    | Manage transaction history.

:ref:`info <info_command_ref-label>`
    | Provide detailed information about installed or available packages.

:ref:`install <install_command_ref-label>`
    | Install packages.

:ref:`leaves <leaves_command_ref-label>`
    | List groups of leaf packages.

:ref:`list <list_command_ref-label>`
    | List installed or available packages.

:ref:`makecache <makecache_command_ref-label>`
    | Generate the metadata cache.

:ref:`mark <mark_command_ref-label>`
    | Change the reason of an installed package.

:ref:`module <module_command_ref-label>`
    | Manage modules.

:ref:`offline <offline_command_ref-label>`
    | Manage offline transactions.

:ref:`provides <provides_command_ref-label>`
    | Find what package provides the given value.

:ref:`reinstall <reinstall_command_ref-label>`
    | Reinstall packages.

:ref:`remove <remove_command_ref-label>`
    | Remove packages.

:ref:`replay <replay_command_ref-label>`
    | Replay stored transactions.

:ref:`repo <repo_command_ref-label>`
    | Manage repositories.

:ref:`repoquery <repoquery_command_ref-label>`
    | Search for packages in repositories.

:ref:`search <search_command_ref-label>`
    | Search for packages using keywords.

:ref:`swap <swap_command_ref-label>`
    | Remove software and install another in the single transaction.

:ref:`system-upgrade <system-upgrade_command_ref-label>`
    | Upgrade the system to a new major release.

:ref:`upgrade <upgrade_command_ref-label>`
    | Upgrade packages.

:ref:`versionlock <versionlock_command_ref-label>`
    | Protect packages from updates to newer versions.


Plugin commands
---------------

Here is the list of the commands available as plugins.
These are available after installing the ``dnf5-plugins`` package.

:ref:`automatic <automatic_plugin_ref-label>`
    | Alternative CLI to ``dnf upgrade`` suitable to be executed automatically and regularly from systemd timers, cron jobs and similar.

:ref:`builddep <builddep_plugin_ref-label>`
    | Install missing dependencies for building an RPM package.

:ref:`changelog <changelog_plugin_ref-label>`
    | Show package changelogs.

:ref:`config-manager <config_manager_plugin_ref-label>`
    | Manages main configuration, repositories configuration, and variables.

:ref:`copr <copr_plugin_ref-label>`
    | Manage Copr repositories (add-ons provided by users/community/third-party).

:ref:`needs-restarting <needs_restarting_plugin_ref-label>`
    | Determine whether the system should be rebooted.

:ref:`repoclosure <repoclosure_plugin_ref-label>`
    | Display a list of unresolved dependencies for repositories.

:ref:`reposync <reposync_plugin_ref-label>`
    | Synchronize packages and metadata of a remote DNF repository to a local directory.


Options
=======

Following options are applicable in the general context for any ``dnf5`` command:

.. _assumeno_option_ref-label:

``--assumeno``
    | Automatically answer no for all questions.

.. _best_option_ref-label:

``--best``
    | Try the best available package versions in transactions.

    Specifically during dnf upgrade, which by default skips over updates that can not be
    installed for dependency reasons, the switch forces ``DNF5`` to only consider the latest
    packages. When running into packages with broken dependencies, ``DNF5`` will fail giving
    the reason why the latest version can not be installed.

    Note that the use of the newest available version is only guaranteed for the packages
    directly requested (e.g. as a command line arguments), and the solver may use older
    versions of dependencies to meet their requirements.

``-C, --cacheonly``
    | Use only cached data for working with packages and repository metadata.
    | Cache won't be updated, even if it is expired.

``--comment=COMMENT``
    | Add a comment to the transaction history.

.. _config_option_ref-label:

``--config=CONFIG_FILE_PATH``
    | Define configuration file location.

``--debugsolver``
    | Dump additional data from solver for debugging purposes.
    | Data are saved in ``./debugdata``.

``--disable-plugin=PLUGIN_NAME,...``
    | Disable specified libdnf5 library plugins for the purpose of the current ``DNF5`` command.
    | This is a list option which can be specified multiple times.
    | Accepted values are names, or a glob of names.

``--disable-repo=REPO_ID,...``
    | Temporarily disable active repositories for the purpose of the current ``DNF5`` command.
    | This is a list option which can be specified multiple times.
    | Accepted values are ids, or a glob of ids.

``--dump-main-config``
    | Print main configuration values to stdout.

``--dump-repo-config=REPO_ID,...``
    | Print repository configuration values to stdout.
    | This is a list option which can be specified multiple times.
    | Accepted values are ids, or a glob of ids.

``--dump-variables``
    | Print variable values to stdout.

``--enable-plugin=PLUGIN_NAME,...``
    | Enable specified libdnf5 library plugins for the purpose of the current ``DNF5`` command.
    | This is a list option which can be specified multiple times.
    | Accepted values are names, or a glob of names.

``--enable-repo=REPO_ID,...``
    | Temporarily enable additional repositories for the purpose of the current ``DNF5`` command.
    | This is a list option which can be specified multiple times.
    | Accepted values are ids, or a glob of ids.

``--forcearch=ARCH``
    | Force the use of a specific architecture.
    | :ref:`See <forcearch_misc_ref-label>` :manpage:`dnf5-forcearch(7)` for more info.

.. _help_option_ref-label:

``-h, --help``
    | Show the help.

``--installroot=ABSOLUTE_PATH``
    | Setup installroot path.
    | Absolute path is required.
    | :ref:`See <installroot_misc_ref-label>` :manpage:`dnf5-installroot(7)` for more info.

.. _no_best_option_ref-label:

``--no-best``
    | Do not limit the transaction to the best candidates only.

``--no-docs``
    | Do not install any files that are marked as a documentation (which includes man pages and texinfo documents).
    | It sets the ``RPMTRANS_FLAG_NODOCS`` flag.

``--no-gpgchecks``
    | Skip checking OpenPGP signatures on packages (if ``RPM`` policy allows that).

``--no-plugins``
    | Disable all libdnf5 plugins.

``-q, --quiet``
    In combination with a non-interactive command, shows just the relevant content.
    Suppresses messages notifying about the current state or actions of ``DNF5``.

``--refresh``
    | Force refreshing metadata before running the command.

``--color=<color>``
    | Control whether color is used in terminal output. Valid values are ``always``, ``never`` and ``auto`` (default).

``--repo=REPO_ID,...``
    | Enable just specified repositories.
    | This is a list option which can be specified multiple times.
    | Accepted values are ids, or a glob of ids.

``--repofrompath=REPO_ID,REPO_PATH``
    Specify a repository to add to the repositories only for this run. Can be used multiple times.

    The new repository id is specified by ``REPO_ID`` and its baseurl by ``REPO_PATH``. Variables in both values are substituted before creating the repo.

    The configuration of the new repository can be adjusted using options ``--setopt=REPO_ID.option=value``.

    If you want only packages from this repository to be available, combine this option with ``--repo=REPO_ID`` switch.

``--releasever=RELEASEVER``
    | Override the value of the distribution release in configuration files.
    | This can affect cache paths, values in configuration files and mirrorlist URLs.

``--setopt=[REPO_ID.]OPTION=VALUE``
    | Override a configuration option from the configuration file.
    | The ``REPO_ID`` parameter is used to override options for repositories.

    Values for the options like ``excludepkgs``, ``includepkgs``, ``installonlypkgs`` and ``tsflags``
    are appended to the original value, they do not override it. However, specifying an empty
    value (e.g. ``--setopt=tsflags=``) will clear the option.

``--setvar=VAR_NAME=VALUE``
    | Override a ``DNF5`` variable value, like ``arch``, ``releasever``, etc.

``--show-new-leaves``
    | Show newly installed leaf packages and packages that became leaves after a transaction.

.. _use_host_config_option_ref-label:

``--use-host-config``
    | Use configuration files and variable definitions from the host system rather than the installroot.
    | :ref:`See <installroot_misc_ref-label>` :manpage:`dnf5-installroot(7)` for more info.

.. _version_option_ref-label:

``--version``
    | Display the version of the dnf5 application and libdnf5 library, along with successfully loaded plugins and their versions, and then exit.

.. _assumeyes_option_ref-label:

``-y, --assumeyes``
    | Automatically answer yes for all questions.

``-x PACKAGE-SPEC-N,..., --exclude=PACKAGE-SPEC-N,...``
    | Exclude packages specified in ``PACKAGE-SPEC-N`` arguments from the transaction.
    | This is a list option.


Metadata Synchronization
========================

Correct operation of ``DNF5`` depends on having an access to up-to-date data from the all enabled
repositories, but contacting remote mirrors on every operation considerably slows it down and costs
bandwidth for both the client and the repository provider. The ``metadata_expire`` repository configuration
option is used by ``DNF5`` to determine whether a particular local copy of repository data is due
to be re-synced. It is crucial that the repository providers set the option well, namely to a value
where it is guaranteed that if particular metadata was available in time ``T`` on the server,
then all packages it references will still be available for download from the server
in time ``T + metadata_expire``.

To further reduce the bandwidth load, some of the commands where having up-to-date metadata
is not critical (e.g. the ``group list`` command) do not look at whether a repository is expired
and whenever any version of it is locally available to the user's account, it will be used.

:ref:`Refer to the <caching_misc_ref-label>` :manpage:`dnf5-caching(7)` for more info regarding
metadata and packages caching.


Configuration Files Replacement Policy
======================================

The updated packages could replace the old modified configuration files with the new ones or keep
the older files. Neither of the files are actually replaced. To the conflicting ones ``RPM``
gives additional suffix to the origin name. Which file should maintain the true name after
transaction is not controlled by package manager, but is specified by each package itself,
following packaging guideline.


Exit Codes
==========

The ``dnf5`` command in general exits with the following return values:

`0`
    | Operation was successful.

`1`
    | An error occurred during processing of the command.

`2`
    | An error occurred during parsing the arguments.

Other exit codes could be returned by the specific command itself, see its documentation for more info.


Files
=====

``Cache Files``
    /var/cache/libdnf5/

``Main Configuration``
    /etc/dnf/dnf.conf

``Repository Configuration``
    /etc/yum.repos.d/

``Repository Persistence``
    /var/lib/dnf/

``System State``
    /usr/lib/sysimage/libdnf5/


Environment
===========

``DNF5_FORCE_INTERACTIVE``
    Override interactivity of the connected standard input/ouput. Set to `0` to pretend
    that the input/output is not connected to an interactive terminal and DNF5
    should not ask any questions. Set to `1` to force DNF5 to ask questions
    even if the terminal is non-interractive. See
    :ref:`--assumeno<assumeno_option_ref-label>` and
    :ref:`--assumeyes<assumeyes_option_ref-label>` options.

``DNF5_PLUGINS_DIR``
    Override a directory with DNF5 application plugins. Set it to an empty
    string to disable loading the application plugins.

``LIBDNF_PLUGINS_CONFIG_DIR``
    Override a directory with libdnf5 plugin's configuration files.

``TEMP``,
``TEMPDIR``,
``TMP``,
``TMPDIR``
    Override a path to a directory for temporary files. The directory must
    exist. Default value is `/tmp`.

See Also
========

Commands:
    | :manpage:`dnf5-advisory(8)`, :ref:`Advisory command <advisory_command_ref-label>`
    | :manpage:`dnf5-autoremove(8)`, :ref:`Autoremove command <autoremove_command_ref-label>`
    | :manpage:`dnf5-check(8)`, :ref:`Check command <check_command_ref-label>`
    | :manpage:`dnf5-check-upgrade(8)`, :ref:`Check-Upgrade command <check-upgrade_command_ref-label>`
    | :manpage:`dnf5-clean(8)`, :ref:`Clean command <clean_command_ref-label>`
    | :manpage:`dnf5-distro-sync(8)`, :ref:`Distro-Sync command <distro-sync_command_ref-label>`
    | :manpage:`dnf5-downgrade(8)`, :ref:`Downgrade command <downgrade_command_ref-label>`
    | :manpage:`dnf5-download(8)`, :ref:`Download command <download_command_ref-label>`
    | :manpage:`dnf5-environment(8)`, :ref:`Environment command <environment_command_ref-label>`
    | :manpage:`dnf5-group(8)`, :ref:`Group command <group_command_ref-label>`
    | :manpage:`dnf5-history(8)`, :ref:`History command, <history_command_ref-label>`
    | :manpage:`dnf5-info(8)`, :ref:`Info command <info_command_ref-label>`
    | :manpage:`dnf5-install(8)`, :ref:`Install command <install_command_ref-label>`
    | :manpage:`dnf5-leaves(8)`, :ref:`Leaves command <leaves_command_ref-label>`
    | :manpage:`dnf5-list(8)`, :ref:`List command <list_command_ref-label>`
    | :manpage:`dnf5-makecache(8)`, :ref:`Makecache command <makecache_command_ref-label>`
    | :manpage:`dnf5-mark(8)`, :ref:`Mark command <mark_command_ref-label>`
    | :manpage:`dnf5-module(8)`, :ref:`Module command <module_command_ref-label>`
    | :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`
    | :manpage:`dnf5-provides(8)`, :ref:`Provides command <provides_command_ref-label>`
    | :manpage:`dnf5-reinstall(8)`, :ref:`Reinstall command <reinstall_command_ref-label>`
    | :manpage:`dnf5-remove(8)`, :ref:`Remove command <remove_command_ref-label>`
    | :manpage:`dnf5-repo(8)`, :ref:`Repo command <repo_command_ref-label>`
    | :manpage:`dnf5-repoquery(8)`, :ref:`Repoquery command <repoquery_command_ref-label>`
    | :manpage:`dnf5-search(8)`, :ref:`Search command <search_command_ref-label>`
    | :manpage:`dnf5-swap(8)`, :ref:`Swap command <swap_command_ref-label>`
    | :manpage:`dnf5-system-upgrade(8)`, :ref:`System-Upgrade command <system-upgrade_command_ref-label>`
    | :manpage:`dnf5-upgrade(8)`, :ref:`Upgrade command <upgrade_command_ref-label>`
    | :manpage:`dnf5-versionlock(8)`, :ref:`Versionlock command <versionlock_command_ref-label>`


Application Plugins:
    | :manpage:`dnf5-automatic(8)`, :ref:`Automatic command <automatic_plugin_ref-label>`
    | :manpage:`dnf5-builddep(8)`, :ref:`Builddep command <builddep_plugin_ref-label>`
    | :manpage:`dnf5-changelog(8)`, :ref:`Changelog command <changelog_plugin_ref-label>`
    | :manpage:`dnf5-config-manager(8)`, :ref:`Config-manager command <config_manager_plugin_ref-label>`
    | :manpage:`dnf5-copr(8)`, :ref:`Copr command <copr_plugin_ref-label>`
    | :manpage:`dnf5-needs-restarting(8)`, :ref:`Needs-Restarting command <needs_restarting_plugin_ref-label>`
    | :manpage:`dnf5-repoclosure(8)`, :ref:`Repoclosure command <repoclosure_plugin_ref-label>`
    | :manpage:`dnf5-repomanage(8)`, :ref:`Repomanage command <repomanage_plugin_ref-label>`
    | :manpage:`dnf5-reposync(8)`, :ref:`Reposync command <reposync_plugin_ref-label>`


Library Plugins:
    | :manpage:`libdnf5-actions(8)`, :ref:`Actions plugin <actions_plugin_ref-label>`
    | :manpage:`libdnf5-expired-pgp-keys(8)`, :ref:`Expired PGP keys plugin <expired-pgp-keys_plugin_ref-label>`

Configuration:
    | :manpage:`dnf5.conf(5)`, :ref:`DNF5 Configuration Reference <dnf5_conf-label>`

Miscellaneous:
    | :manpage:`dnf5-aliases(7)`, :ref:`Aliases for command line arguments <aliases_misc_ref-label>`
    | :manpage:`dnf5-caching(7)`, :ref:`Caching <caching_misc_ref-label>`
    | :manpage:`dnf5-comps(7)`, :ref:`Comps groups and environments <comps_misc_ref-label>`
    | :manpage:`dnf5-filtering(7)`, :ref:`Packages filtering, <filtering_misc_ref-label>`
    | :manpage:`dnf5-forcearch(7)`, :ref:`Forcearch parameter <forcearch_misc_ref-label>`
    | :manpage:`dnf5-installroot(7)`, :ref:`Installroot parameter <installroot_misc_ref-label>`
    | :manpage:`dnf5-modularity(7)`, :ref:`Modularity overview, <modularity_misc_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
    | :manpage:`dnf5-system-state(7)`, :ref:`System state <systemstate_misc_ref-label>`

Project homepage:
    | https://github.com/rpm-software-management/dnf5
