====================================
Changes in DNF5 in comparison to DNF
====================================

The chapter describe differences between ``DNF5`` (https://github.com/rpm-software-management/dnf5) and ``DNF``
(https://github.com/rpm-software-management/dnf)

Changes on the API:
===================
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


Changes on the command line:
============================

Commands cannot have optional subcommands and optional arguments. Optional subcommands were ambiguous, it wasn't clear
whether the input is a command argument or a subcommand. If present, subcommands are now mandatory.
``dnf history <transaction ID>`` -> ``dnf history info <transaction ID>``
``dnf updateinfo`` -> ``dnf updateinfo summary``

Options that cannot be applied to all commands or can be applied but without any effect should be removed from general
options and implemented only for related commands.
For example: ``--best``, ``--nobest`` are related to only several transaction commands.

Renaming boolean options to format ``--<option>``, and ``--no-<option>``
``--nobest`` -> ``--no-best``. Proposing to keep ``--nobest`` as a deprecated option.

To disable all configuration file excludes, the ``*`` glob character is used now instead of the ``all`` within
the ``disable_excludes`` configuration option to unify the behavior with query objects on the API.

strict configuration option deprecation
---------------------------------------
``strict`` config option is now deprecated. The problem with this option is that it does two things:

 1. if disabled it allows the solver to skip uninstallable packages to resolve depsolv problems
 2. if disabled it allows dnf to skip unavailable packages (this is for ``install`` command mostly)

The functionality is now split to two config options - ``skip_broken`` for the uninstallable packages and
``skip_unavailable`` for packages not present in repositories. Together with these new config options there are also
corresponding command line options ``--skip-broken`` and ``--skip-unavailable`` for commands where it makes sense.


Global options
--------------
* Options ``--disable-repo=REPO_ID`` and ``--setopt=[REPO_ID.]OPTION=VALUE`` now always cause an error when provided with invalid ``REPO_ID``.
  This makes them consistent with ``--repo=REPO_ID`` and ``--enable-repo=REPO_ID``. The ``strict`` configuration option is no longer taken into account.
* Option ``--help-cmd`` is dropped as we already have ``--help`` and ``-h`` options.

Alias command
-------------
* Dropped. The command is replaced by a different functionality, see
  :ref:`Aliases for command line arguments <aliases_misc_ref-label>`.

Autoremove command
------------------
 * Dropped ``<spec>`` positional argument since the usecase is sufficiently covered by the ``remove`` command.
 * Specific ``autoremove-n``, ``autoremove-na``, and ``autoremove-nevra`` variants of the command are not supported.

Distro-sync command
-------------------
 * When any argument does not match any package or it is not installed, DNF5 fail. The behavior can be modified by
   the ``--skip-unavailable`` option.
 * Dropped ``distrosync`` and ``distribution-synchronization`` aliases

Downgrade command
-----------------
 * When any argument does not match any package or it is not installed, DNF5 fail. The behavior can be modified by
   the ``--skip-unavailable`` option.

Download command
----------------
 * Option ``--source`` was renamed to ``--srpm``.

Group command
-------------
 * Dropped ``group mark install`` and ``group mark remove`` subcommands in favour of the
   new ``--no-packages`` option of the ``group install/remove`` commands. So for example
   to mark a group as installed without touching any packages,
   ``dnf5 group install --no-packages <group_id>`` command can be used.
 * Dropped ``groupinstall`` alias. It is replaced by ``dnf group install``
 * Dropped ``groupinfo`` alias. It is replaced by ``dnf group info``
 * Dropped ``grouplist`` alias. It is replaced by ``dnf group list``
 * Dropped ``grouperase`` alias. It is replaced by ``dnf group remove``
 * Dropped ``groupremove`` alias. It is replaced by ``dnf group remove``
 * Dropped ``groupupdate`` alias. It is replaced by ``dnf group upgrade``
 * Dropped ``groups`` alias. It is replaced by ``dnf group``

Help command
------------
 * Dropped. The functionality is replaced by ``--help`` option

Info command
------------
 * Dropped ``if`` alias.

List command
------------
 * Dropped ``--all`` option since this behavior is now the default one.
 * Changed the list of ``--available`` packages. Previously, dnf4 only listed packages that are either not installed, or
   whose version is higher than the installed version. Now this behaviour is kept when no modifier is used - to skip
   packages already listed in the ``Installed Packages`` section to reduce duplicities. But if the ``--available`` modifier
   is used, dnf5 considers all versions available in the enabled repositories, regardless of which version is installed.

Makecache command
-----------------
 * Metadata is stored in different directories now, see :ref:`cachedir changes`.

Module command
--------------
 * Dropped ``--all`` option since this behavior is the default one.

Needs-restarting command
------------------------
 * ``needs-restarting`` no longer scans for open files to determine whether any outdated files are still in use. The default behavior is now the ``--reboothint`` behavior of DNF 4 needs-restarting, which reports whether a system reboot is recommended depending on which packages have been updated since the most recent boot.
 * Reboot will now be recommended if any package with an associated ``reboot_suggested`` advisory has been installed or updated.
 * The ``-s, --services`` option no longer scans for open files. Instead, restarting a service is recommended if any dependency of the package that provides the service, or the package itself, has been updated since the service started.
 * Dropped ``-r, --reboothint`` option; this is now the default behavior.
 * Dropped ``-u, --useronly`` option.

Offline-distrosync command
--------------------------
 * Now an alias of ``dnf5 distro-sync --offline``

Offline-upgrade command
--------------------------
 * Now an alias of ``dnf5 upgrade --offline``

Remove command
--------------
 * Command does not remove packages according to provides, but only according NEVRA or file provide match
 * Dropped commands ``remove-n``, ``remove-na``, ``remove-nevra``.
 * Dropped erase aliases for the same ``erase``, ``erase-n`` , ``erase-na`` , ``erase-nevra``.

Repoclosure command
-------------------
 * Dropped ``--pkg`` option. Positional arguments can be used to specify packages to check closure for.

Repolist command
----------------
 * Option ``-v`` and ``--verbose`` were removed. The functionality is replaced by ``repoinfo`` command that was already
   introduced in DNF4.

Repoquery command
-----------------
 * Dropped: ``-a/--all``, ``--alldeps``, ``--nevra`` options, their behavior is and has been the default for both dnf4 and
   dnf5. The options are no longer needed.
 * Dropped: ``--nvr``, ``--envra`` options. They are no longer supported.
 * Dropped: ``--archlist`` alias for ``--arch``.
 * Dropped: ``-f`` alias for ``--file`` also the arguments to ``--file`` are separated by comma instead of a space.
 * Moved ``--groupmember`` option to the Group info and list commands and renamed to ``--contains-pkgs``.
 * --queryformat/--qf no longer prints additional new line at the end of each formatted string, bringing it closer to
   rpm --query behavior.
 * --queryformat no longer supports ``size`` tag because it was printing install size for installed packages and download
   size for not-installed packages. This could be confusing.
 * Option ``--source`` was renamed to ``--sourcerpm`` and it now matches queryformat's ``sourcerpm`` tag.
 * Option ``--resolve`` was changed to ``--providers-of=PACKAGE_ATTRIBUTE``. It no longer interacts with the formatting ``--requires``,
   ``--provides``, ``--suggests``,... options instead it takes the PACKAGE_ATTRIBUTE value directly.
   E.g., ``dnf rq --resolve --requires glibc`` -> ``dnf rq --providers-of=requires glibc``.

System-upgrade command
--------------------------
 * Moved from a plugin to a built-in command

Upgrade command
---------------
 * New dnf5 option ``--minimal`` (``upgrade-minimal`` command still exists as a compatibility alias for
   ``upgrade --minimal``).
 * When any argument does not match any package or it is not installed, DNF5 fail. The behavior can be modified by
   the ``--skip-unavailable`` option.
 * Dropped upgrade command aliases ``upgrade-to`` and ``localupdate``.
 * Dropped ``--skip-broken`` option, as it was already available in DNF4 only for compatibility reasons with YUM,
   but it has no effect. Instead, the decision about selecting the newer version of a package into the transaction
   and skipping possible dependency issues is based on the :ref:`best <best_option_ref-label>` or
   :ref:`no-best <no_best_option_ref-label>` option.

Updateinfo command
------------------
 * The command has been renamed to ``advisory`` (but there is a compatibility ``updateinfo`` alias).
 * It is required to always specify a subcommand: ``dnf updateinfo`` -> ``dnf5 advisory summary``.
 * Options ``--summary``, ``--list`` and ``--info`` have been changed to subcommands. See ``dnf5 advisory --help``.
 * Option ``--sec-severity`` (``--secseverity``) has been renamed to ``--advisory-severities=ADVISORY_SEVERITY,...``.
 * The ``advisory`` commands now accept only advisory ID, in order to filter by packages use ``--contains-pkgs=PACKAGE_NAME,...`` option.
 * Dropped deprecated aliases: ``list-updateinfo``, ``list-security``, ``list-sec``, ``info-updateinfo``, ``info-security``, ``info-sec``, ``summary-updateinfo``.
 * Dropped ``upif`` alias.

Changes of configuration:
=========================

Default of ``best`` configuration option changed to ``true``
------------------------------------------------------------
The new default value ensures that important updates will not be skipped and issues in distribution will be reported
earlier.

.. _cachedir changes:

cachedir and system_cachedir options
------------------------------------
The default root cache directory (``system_cachedir``) is now ``/var/cache/libdnf5``, while for users, the ``cachedir``
is at ``/home/$USER/.cache/libdnf5``. Users no longer access the root's cache directly; instead, metadata is copied
to the user's location if it's empty or invalid. For additional information, refer to the :ref:`Caching <caching_misc_ref-label>` man page.

cacheonly option
----------------
The ``cacheonly`` option was changed from ``bool`` to ``enum`` with options ``all``, ``metadata`` and ``none``,
enabling users to specify whether to use the cache exclusively for metadata or for both metadata and packages.
