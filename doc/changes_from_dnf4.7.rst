.. _changes_ref-label:

#############################
 Changes between DNF and DNF5
#############################

This chapter describes the differences between `DNF5 <https://github.com/rpm-software-management/dnf5>`_ and `DNF <https://github.com/rpm-software-management/dnf>`_.

.. _cli_changes_ref-label:

Changes on the CLI
==================

Options
-------

Global options scoping
^^^^^^^^^^^^^^^^^^^^^^
Options that cannot be applied to all commands or may be applicable but have no effect are removed from general options and implemented only for related commands.

Examples: ``--best``, ``--no-best`` are only relevant to several transaction commands.


Options renaming
^^^^^^^^^^^^^^^^
Renaming boolean options to the following formats:

  * ``--<option>`` and ``--no-<option>``
  * ``--enable-<option>`` and ``--disable-<option>``

The options with original names are retained for now as compatibility aliases.

Examples: ``--best`` and ``--no-best``.


Strict behavior
^^^^^^^^^^^^^^^
  * Options ``--disable-repo=REPO_ID`` and ``--setopt=[REPO_ID.]OPTION=VALUE`` now consistently result in an error when provided with an invalid ``REPO_ID``.
  * The behavior is now aligned with the ``--repo=REPO_ID`` and ``--enable-repo=REPO_ID``.
  * The ``strict`` configuration option is no longer considered, see the :ref:`strict option deprecation <strict_option_conf_changes_ref-label>` for more information.


Changes to individual options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
``-4/-6``
  * Dropped. Now only the ``ip_resolve`` configuration option is available.

``--color``
  * Dropped. Now only the ``color`` configuration option is available.

.. TODO(jkolarik): Not implemented yet
   ``-d, --debuglevel``
     * Dropped. Now only the ``debuglevel`` configuration option is available.

``--disableexcludes`` and ``--disableexcludepkgs``
  * Dropped. Now only the ``disable_excludes`` configuration option is available.

``--disable, --enable``
  * Dropped along with the previously existing alternatives ``--set-disabled`` and ``--set-enabled``.
  * Currently, this can only be achieved through the :ref:`config-manager <config_manager_plugin_ref-label>` plugin.

    * For example, to disable the ``fedora`` repository: ``dnf config-manager setopt fedora.enabled=0``.

``--downloaddir``
  * Dropped. Now only the ``--destdir`` is used for the ``download`` command.
  * When downloading packages using the ``system-upgrade`` or ``offline`` command, the target path construction now utilizes the configured ``installroot`` and ``cachedir`` options.

``-e, --errorlevel``
  * Both the ``--errorlevel`` option and ``errorlevel`` configuration option are dropped.

``--help-cmd``
  * Dropped. Now only the ``-h`` or ``--help`` options are available.

``--installroot``
  * New behavior introduced to define from which place the configuration and variables are loaded.
  * See the :ref:`installroot documentation <installroot_misc_ref-label>` for more information.

``--noautoremove``
  * Applicable only for the ``remove`` command now. As a workaround for other commands, you can use ``--setopt=clean_requirements_on_remove=False``.

``--obsoletes``
  * Dropped. Now only the ``obsoletes`` configuration option is available.

``-R, --randomwait``
  * Dropped.

``--rpmverbosity``
  * Dropped. Now only the ``rpmverbosity`` configuration option is available.

``--sec-severity``
  * Renamed to ``--advisory-severities``.

``-v, --verbose``
  * Not implemented at present. May be added for specific commands in the future.

``--version``
  * Behavior is different now. See the :ref:`main man page <version_option_ref-label>` for more details.


Newly introduced options
^^^^^^^^^^^^^^^^^^^^^^^^
``--allow-downgrade``
  * Along with ``--no-allow-downgrade``, these options enable/disable the downgrade of dependencies when resolving transactions.
  * New respective configuration options have also been created.
  * Applicable to ``install``, ``upgrade``, and related commands.

``--dump-main-config``
  * Along with related ``--dump-repo-config=REPO_ID``, these are new options to print configuration values on the standard output.

``--offline``
  * Store the transaction to be performed offline.
  * Applicable to all relevant transactional commands.
  * See the :ref:`Offline command <offline_command_ref-label>` for more information.

``--show-new-leaves``
  * Show newly installed leaf packages and packages that became leaves after a transaction.

``--skip-unavailable``
  * Allow skipping packages that are not available in repositories.
  * Not to be confused with the :ref:`skip_if_unavailable <skip_if_unavailable_options-label>` configuration option.
  * Applicable to ``install``, ``upgrade``, and related commands.
  * See also the :ref:`strict option deprecation <strict_option_conf_changes_ref-label>` for more information.

``--use-host-config``
  * See the :ref:`main man page <use_host_config_option_ref-label>` for more details.


Commands
--------

Optional subcommands
^^^^^^^^^^^^^^^^^^^^
Commands cannot have optional subcommands. Optional subcommands were ambiguous,
making it unclear whether the input was intended as a command argument or a subcommand. Subcommands are now mandatory if present.

Examples:
  * Before: ``dnf history <transaction ID>`` Now: ``dnf history info <transaction ID>``
  * Before: ``dnf updateinfo`` Now: ``dnf updateinfo summary``


Changes to individual commands
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
``alias``
  * Dropped. The command has been replaced by a different functionality.
  * See the :ref:`Aliases for command line arguments <aliases_misc_ref-label>` for more information.

``automatic``
  * Now a DNF5 plugin.
  * The specific systemd units, ``dnf-automatic-download``, ``dnf-automatic-install``, and ``dnf-automatic-notifyonly``, have been dropped. Only one ``dnf5-automatic`` timer is shipped.
  * See the :ref:`Automatic command <automatic_plugin_ref-label>` for more information.

``autoremove``
  * Dropped the ``<spec>`` positional argument since the use case is sufficiently covered by the ``remove`` command.
  * Specific variants of the command, ``autoremove-n``, ``autoremove-na``, and ``autoremove-nevra``, are not supported anymore.

``builddep``
  * Dropped ``--spec`` and ``--srpm`` arguments as automatic detection from file extensions is implemented now.

``config-manager``
  * New behavior introduced.
  * Parameters are replaced by subcommands.

    * Examples:

      * Before: ``--add-repo`` Now: ``addrepo``
      * Before: ``--save --setopt`` Now: ``setopt``

  * Existing repository files are not modified; drop-in override files are created instead.
  * See the :ref:`config-manager documentation <config_manager_plugin_ref-label>` for more information.

``debuginfo-install``
  * Now does not support `autoupdate` functionality. The permanent enablement of debug repositories can be achieved
    using `config-manager` command.
``distro-sync``
  * Now when any argument doesn't match an installed package, DNF5 fails. The behavior can be modified by the ``--skip-unavailable`` option.
  * Dropped ``distrosync`` and ``distribution-synchronization`` aliases.

``downgrade``
  * Now when any argument doesn't match an installed package, DNF5 fails. The behavior can be modified by the ``--skip-unavailable`` option.

``download``
  * Option ``--source`` was renamed to ``--srpm``.
  * Dropped ``--downloaddir`` argument, only ``--destdir`` is supported now.

``group``
  * New option ``--contains-pkgs`` to filter only groups containing specific packages.
  * Dropped ``--ids`` argument as group ids are always in the output now.
  * Dropped ``group mark install`` and ``group mark remove`` subcommands in favor of the new ``--no-packages`` option for the ``install`` and ``remove`` commands.

    * E.g. to mark a group as installed without touching any packages, ``dnf5 group install --no-packages <group_id>`` command can be used.

  * Dropped ``groupinstall`` alias. It is replaced by ``dnf group install``.
  * Dropped ``groupinfo`` alias. It is replaced by ``dnf group info``.
  * Dropped ``grouplist`` alias. It is replaced by ``dnf group list``.
  * Dropped ``grouperase`` alias. It is replaced by ``dnf group remove``.
  * Dropped ``groupremove`` alias. It is replaced by ``dnf group remove``.
  * Dropped ``groupupdate`` alias. It is replaced by ``dnf group upgrade``.
  * Dropped ``groups`` alias. It is replaced by ``dnf group``.

``help``
  * Dropped. The functionality is replaced by the ``--help`` option.

``history``
  * ``undo`` subcommand now accepts ``--ignore-extras`` and ``--ignore-installed`` like original ``history replay`` command.
  * ``store`` subcommand now creates a directory with transaction JSON file instead of a single transaction JSON file directly.
  * ``store`` subcommand's ``--output`` option now accepts a directory path instead of a file. The default is ``./transaction``.
  * ``replay`` subcommand was moved to a standalone ``replay`` command, that now accepts a path to a directory instead of a file path.
    The directory can be created with ``--store`` option and in addition to the JSON transaction, it can contain packages, group and environments used in the transaction.

``info``
  * Dropped ``--all`` option since this behavior is the default one.
  * Dropped ``--updates`` option, only ``--upgrades`` is available now.

``list``
  * Dropped ``--all`` option since this behavior is the default one.
  * Changed the behavior of the ``--available`` option.
    * In DNF4, only packages not installed or with higher versions were listed. This behavior remains unchanged when the option is not used, reducing duplications in the "Installed Packages" section.
    * When using the ``--available`` option, DNF5 considers all versions available in enabled repositories, irrespective of the installed version.

``makecache``
  * Metadata is now stored in different directories, see the ``cachedir`` configuration option :ref:`changes <cachedir_option_conf_changes_ref-label>` for more details.

``mark``
  * Renaming subcommands to be more intuitive: ``install`` -> ``user``, ``remove`` -> ``dependency``.
  * New ``weak`` subcommand to mark a package as a weak dependency.
  * Now when any argument doesn't match an installed package, DNF5 fails. The behavior can be modified by the ``--skip-unavailable`` option.

``module``
  * Dropped ``--all`` option since this behavior is the default one.

``needs-restarting``
  * Command no longer scans for open files to determine outdated files still in use. The default behavior now aligns with DNF 4's ``--reboothint``, suggesting a system reboot depending on updated packages since the last boot.
  * Reboot recommendations are now triggered if any package with a ``reboot_suggested`` advisory has been installed or updated.
  * The ``-s, --services`` option no longer scans for open files. Instead, restarting a service is recommended if any dependency of the service-providing package or the package itself has been updated since the service started.
  * Dropped ``-r, --reboothint`` option since this behavior is now the default one.
  * Dropped ``-u, --useronly`` option.

``offline-distrosync``
  * Now it's an alias of ``dnf5 distro-sync --offline``.

``offline-upgrade``
  * Now it's an alias of ``dnf5 upgrade --offline``.

``remove``
  * Command no longer removes packages according to provides, but only based on NEVRA or file provide match.
  * Dropped commands ``remove-n``, ``remove-na``, ``remove-nevra``.
  * Specific variants of the command, ``remove-n``, ``remove-na``, and ``remove-nevra``, are not supported anymore.

    * Dropped also the related aliases, ``erase``, ``erase-n``, ``erase-na`` and ``erase-nevra``.

``repoclosure``
  * Dropped ``--pkg`` option. Positional arguments can now be used to specify packages to check closure for.

``reposync``
  * Dropped ``--downloadcomps`` option. Consider using ``--download-metadata`` option which downloads all available repository metadata, not only comps groups.

``repolist``
  * The ``repolist`` and ``repoinfo`` commands are now subcommands of the ``repo`` command: ``repo list`` and ``repo info``.

    * Original commands still exist as compatibility aliases.

  * Options ``-v`` and ``--verbose`` have been removed. The functionality is replaced by the ``repo info`` command (already in DNF4 as ``repoinfo``).
  * When no repositories are configured, empty output is now provided instead of displaying "No repositories available".

``repoquery``
  * Dropped: ``-a/--all``, ``--alldeps``, ``--nevra`` options. Their behavior is and has been the default for both DNF4 and DNF5, so the options are no longer needed.
  * Dropped: ``--envra``, ``--nvr``, ``--unsatisfied`` options. They are no longer supported.
  * Dropped: ``--archlist`` alias for ``--arch``.
  * Dropped: ``-f`` alias for ``--file``. Also, the arguments to ``--file`` are separated by commas instead of spaces.
  * Moved ``--groupmember`` option to the ``info`` and ``list`` subcommands of the ``group`` and ``advisory`` commands, renaming it to ``--contains-pkgs``.
  * ``--queryformat, --qf`` no longer prints an additional newline at the end of each formatted string, bringing it closer to the behavior of ``rpm --query``.
  * ``--queryformat`` no longer supports ``size`` tag because it was printing install size for installed packages and download size for not-installed packages, which could be confusing.
  * Option ``--source`` was renamed to ``--sourcerpm``, and it now matches queryformat's ``sourcerpm`` tag.
  * Option ``--resolve`` was changed to ``--providers-of=PACKAGE_ATTRIBUTE``. It no longer interacts with the formatting options such as ``--requires``, ``--provides``, ``--suggests``, etc. Instead, it takes the PACKAGE_ATTRIBUTE value directly.

    * For example, ``dnf rq --resolve --requires glibc`` is now ``dnf rq --providers-of=requires glibc``.

  * See the :ref:`Repoquery command <repoquery_command_ref-label>` for more information.

``system-upgrade``
  * Moved from a plugin to a built-in command.

``upgrade``
  * New option ``--minimal``.

    * ``upgrade-minimal`` still exists as a compatibility alias for ``upgrade --minimal``.

  * Now when any argument doesn't match an installed package, DNF5 fails. The behavior can be modified by the ``--skip-unavailable`` option.
  * Dropped ``upgrade-to`` and ``localupdate`` aliases.
  * Dropped ``--skip-broken`` option, as it was already available in DNF4 only for compatibility reasons with YUM, but has no effect.

    * Instead, decisions about package selection and handling dependency issues are based on the :ref:`best <best_option_ref-label>` or :ref:`no-best <no_best_option_ref-label>` options.

``updateinfo``
  * Renamed the command to ``advisory``

    * ``updateinfo`` still exists as a compatibility alias.

  * Subcommands are now mandatory: ``dnf updateinfo`` is now ``dnf5 advisory summary``.
  * Options ``--summary``, ``--list`` and ``--info`` have been changed to subcommands. See ``dnf5 advisory --help``.
  * Option ``--sec-severity`` has been renamed to ``--advisory-severities=ADVISORY_SEVERITY,...``.
  * The ``advisory`` commands now only accept advisory IDs; to filter by packages, use the ``--contains-pkgs=PACKAGE_NAME,...`` option.
  * Dropped deprecated aliases: ``list-updateinfo``, ``list-security``, ``list-sec``, ``info-updateinfo``, ``info-security``, ``info-sec``, ``summary-updateinfo``.
  * Dropped ``upif`` alias.

``versionlock``
  * New format of the configuration file.
  * See the :ref:`Versionlock command <versionlock_command_ref-label>` for more information.

.. _api_changes_ref-label:

Changes on the API
==================

PackageSet::operator[]
----------------------
It was removed due to insufficient O(n^2) performance.
Use PackageSet iterator to access the data instead.


Package::get_epoch()
--------------------
The return type was changed from ``unsigned long`` to ``std::string``.


DNF: Package.size, libdnf: dnf_package_get_size()
-------------------------------------------------
The return value was ambiguous, returning either package or install size.
Use Package::get_download_size() and Package::get_install_size() instead.


dnf_sack_set_installonly, dnf_sack_get_installonly, dnf_sack_set_installonly_limit, dnf_sack_get_installonly_limit
------------------------------------------------------------------------------------------------------------------
The functions were dropped as unneeded. The installonly packages are taken directly from main Conf in Base.


Query::filter() - HY_PKG_UPGRADES_BY_PRIORITY, HY_PKG_OBSOLETES_BY_PRIORITY, HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
---------------------------------------------------------------------------------------------------------------
The priority filter was separated into a standalone method.
Combine ``query.filter_priority()`` with ``query.filter_latest_evr()`` or another filter to achieve the original
functionality.


Query::filter() - HY_PKG_LATEST
-------------------------------
The filter was replaced with ``filter_latest_evr()`` which has the same behavior as ``HY_PKG_LATEST_PER_ARCH``


ConfigMain::proxy_auth_method() and ConfigRepo::proxy_auth_method()
-------------------------------------------------------------------
The return types were changed. ``OptionEnum<std::string>`` was replaced by ``OptionStringSet``.
A combination of several authentication methods (for example "basic" and "digest") can now be used.
This allows using a list of authentication methods in configuration files and the DNF5 command line
"--setopt=proxy_auth_method=".


.. _conf_changes_ref-label:

Changes to configuration
========================

.. _strict_option_conf_changes_ref-label:

Deprecation of the ``strict`` option
------------------------------------
``strict`` configuration option is now deprecated due to its dual functionality:

 1. It allows the solver to skip uninstallable packages to resolve dependency problems.
 2. It permits DNF to skip unavailable packages (mostly for the ``install`` command).

To address this, the functionality has been split into two configuration options:

  * ``skip_broken`` for uninstallable packages.
  * ``skip_unavailable`` for packages not present in repositories.

Additionally, corresponding command-line options ``--skip-broken`` and ``--skip-unavailable`` have been introduced for commands where applicable.


Changes to individual options
-----------------------------
``best``
  * Default value is changed to ``true``.
  * The new default value ensures that important updates will not be skipped and issues in distribution will be reported earlier.

.. _cachedir_option_conf_changes_ref-label:

``cachedir``
  * The default user cached dir is now at ``~/.cache/libdnf5``.
  * The default root cache directory, configured by the ``system_cachedir`` option, is now ``/var/cache/libdnf5``.
  * Users no longer access the root's cache directly; instead, metadata is copied to the user's location if it's empty or invalid.
  * For additional information, refer to the :ref:`Caching <caching_misc_ref-label>` man page.

``cacheonly``
  * The option was changed from ``bool`` to ``enum`` with options ``all``, ``metadata`` and ``none``.

    * This enables users to specify whether to use the cache exclusively for metadata or for both metadata and packages.

``deltarpm``
  * Default value is changed to ``false``.
  * The support for delta RPMs is not implemented for now.

``disable_excludes``
  * To disable all configuration file excludes, the ``*`` glob character is used now instead of the ``all`` to unify the behavior with query objects on the API.

``keepcache``
  * The behavior has been slightly modified, see the :ref:`Caching <caching_packages_ref-label>` man page for more information.

``optional_metadata_types``
  * Default value is now: ``comps,updateinfo``.
  * Supported values are now extended to the following list: ``comps``, ``filelists``, ``other``, ``presto``, ``updateinfo``.


Newly introduced configuration options
--------------------------------------
``allow_downgrade``
  * New option used to enable or disable downgrade of dependencies when resolving transaction.

``skip_broken``, ``skip_unavailable``, ``strict``
  * New options ``skip_broken``, ``skip_unavailable`` were added due to deprecation of ``strict`` option.
  * See the :ref:`strict deprecation <strict_option_conf_changes_ref-label>` above.


Dropped configuration options
-----------------------------
``arch`` and ``basearch``
  * It is no longer possible to change the detected architecute in configuration files.
  * See the :manpage:`dnf5-forcearch(7)`, :ref:`Forcearch parameter <forcearch_misc_ref-label>` for overriding architecture.

``errorlevel``
  * The option was deprecated in dnf < 5 and is dropped now.
