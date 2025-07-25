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

.. _dnf5_conf-label:

##############################
 DNF5 Configuration Reference
##############################

Description
===========

`DNF5` by default uses the global configuration file at /etc/dnf/dnf.conf.

The configuration file has INI format consisting of section declaration and name=value options below each on separate
line. There are two types of sections in the configuration files: main and repository.  Main  section  defines all
global configuration options and should be only one.

The repository  sections  define  the configuration for each (remote or local) repository. The section name of the
repository in brackets serve as repo ID reference and should be unique  across  configuration  files.  The  allowed
characters  of  repo  ID  string  are  lower and upper case alphabetic letters, digits, -, _, .  and :. The minimal
repository configuration file should aside from repo ID consists of baseurl, metalink or mirrorlist option  definition.

.. _conf_main_options-label:

[main] Options
==============

.. _allow_downgrade_options-label:

``allow_downgrade``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 allows downgrading packages while resolving dependencies.

    Default: ``True``.

.. _allow_vendor_change_options-label:

``allow_vendor_change``
    :ref:`boolean <boolean-label>`

    If disabled DNF5 will stick to vendor when upgrading or downgrading rpms.

    Default: ``True``.

    .. WARNING:: This option is currently not supported for `downgrade` and `distro-sync` commands

.. _assumeno_options-label:

``assumeno``
    :ref:`boolean <boolean-label>`

    If enabled DNF5 will assume ``No`` where it would normally prompt for confirmation from user input

    Default: ``False``.

.. _assumeyes_options-label:

``assumeyes``
    :ref:`boolean <boolean-label>`

    If enabled DNF5 will assume ``Yes`` where it would normally prompt for
    confirmation from user input (see also :ref:`defaultyes
    <defaultyes_options-label>`).

    Default: ``False``.

.. _best_options-label:

``best``
    :ref:`boolean <boolean-label>`

    If ``True``, instructs the solver to either use a package with the highest
    available version or fail. If ``False``, do not fail if the latest version
    cannot be installed and go with the lower version.

    Default: ``True``.

    .. NOTE::
       This option in particular :ref:`can be set in your configuration file by
       your distribution <conf_distribution_specific_options-label>`.

.. _cachedir_options-label:

``cachedir``
    :ref:`string <string-label>`

    Path to a directory used by various DNF5 subsystems for storing cache data
    including repositories metadata and downloaded packages.
    Has a reasonable root-writable default depending on the distribution. DNF5
    needs to be able to create files and directories at this location.

    Regular user default: ``~/.cache/libdnf5``.

    For superuser the value is overwritten by :ref:`system_cachedir <_system_cachedir_options-label>` option.

.. _cacheonly_options-label:

``cacheonly``
    :ref:`string <string-label>`

    Can be ``all``, ``metadata``, ``none``.

    If set to ``all`` DNF5 will run entirely from system cache, will not update
    the cache and will use the system cache even if it is expired.

    If set to ``metadata`` DNF5 will cache metadata only.

    Default: ``none``.

    .. NOTE::
       API Notes: Must be set before repository objects are created. Plugins must set
       this in the pre_config hook. Later changes are ignored.

.. _check_config_file_age_options-label:

``check_config_file_age``
    :ref:`boolean <boolean-label>`

    If enabled DNF5 should automatically expire metadata of repos, which are older than
    their corresponding configuration file (usually the dnf.conf file and the foo.repo file).

    Default: ``True``.

    .. NOTE::
       Expire of metadata is also affected by metadata age. See also

       :ref:`metadata_expire <metadata_expire_options-label>`.

.. _clean_requirements_on_remove_options-label:

``clean_requirements_on_remove``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will remove dependencies that are no longer used during ``dnf remove``.
    A package only qualifies for removal via ``clean_requirements_on_remove`` if it was
    installed through DNF5 but not on explicit user request, i.e. it was pulled in as a dependency.

    :ref:`installonlypkgs <installonlypkgs_options-label>` are never automatically removed.

    Default: ``True``.

.. _debugdir_options-label:

``debugdir``
    :ref:`string <string-label>`

    Location where libsolv debug files will be created when enabled
    by :ref:`debug_solver <_debug_solver_options-label>`.

    Default `./debugdata`.

.. _debug_solver_options-label:

``debug_solver``
    :ref:`boolean <boolean-label>`

    If enabled, libsolv debug files will be created when solving the
    transaction. The debug files are created in the :ref:`debugdir <_debugdir_options-label>` directory.

    Default: ``False``.

.. _defaultyes_options-label:

``defaultyes``
    :ref:`boolean <boolean-label>`

    If enabled, the default answer to user confirmation prompts will be ``Yes``.
    Not to be confused with :ref:`assumeyes <assumeyes_options-label>` which will not prompt at all.

    Default: ``False``.

.. _destdir_options-label:

``destdir``
    :ref:`string <string-label>`

    Redirect downloaded packages to provided directory.

    Default: <package repository :ref:`cachedir <cachedir_options-label>`>/packages

.. _exclude_from_weak_options-label:

``exclude_from_weak``
    :ref:`list <list-label>`

    Prevent installing packages as weak dependencies (recommends or
    supplements). The packages can be specified by a name or a glob and
    separated by a comma.

    Defaults to [].

.. _exclude_from_weak_autodetect_options-label:

``exclude_from_weak_autodetect``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will autodetect unmet weak dependencies (recommends or
    supplements) of packages installed on the system. Providers of these weak
    dependencies will not be installed by dnf as weak dependencies any more
    (they will still be installed if pulled in as a regular dependency).

    Defaults to true.

.. _excludeenvs_options-label:

``excludeenvs``
    :ref:`list <list-label>`

    Exclude environments, specified by an id or a glob and separated by a comma, from all operations.

    Can be disabled using ``disable_excludes`` config option.

    Default: ``[]``.

.. _excludegroups_options-label:

``excludegroups``
    :ref:`list <list-label>`

    Exclude groups, specified by an id or a glob and separated by a comma, from all operations.

    Can be disabled using ``disable_excludes`` config option.

    Default: ``[]``.

.. _group_package_types_options-label:

``group_package_types``
    :ref:`list <list-label>`

    List of the following: ``optional``, ``default``, ``mandatory`` or ``conditional``.

    Tells DNF5 which type of packages in groups will be installed when 'group install <group-spec>'
    or 'install @<group-spec>' is called.

    Default: ``default, mandatory, conditional``.

.. _ignorearch_options-label:

``ignorearch``
    :ref:`boolean <boolean-label>`

    If enabled, RPM will allow attempts to install packages incompatible with the CPU's architecture.

    Default: ``False``.

.. _installonlypkgs_options-label:

``installonlypkgs``
    :ref:`list <list-label>`

    List of provide names of packages that should only ever be installed, never
    upgraded. Kernels in particular fall into this category.
    These packages are never removed by ``dnf autoremove`` even if they were
    installed as dependencies (see
    :ref:`clean_requirements_on_remove <clean_requirements_on_remove_options-label>`
    for auto removal details).
    This option append the list values to the default installonlypkgs list used
    by DNF5. The number of kept package versions is regulated
    by :ref:`installonly_limit <installonly_limit_options-label>`.

.. _installonly_limit_options-label:

``installonly_limit``
    :ref:`integer <integer-label>`

    Number of :ref:`installonly packages <installonlypkgs_options-label>` allowed to be installed
    concurrently.

    ``1`` is explicitly not allowed since it complicates kernel upgrades due to protection of
    the running kernel from removal.

    Minimum is ``2``.

    ``0`` means unlimited number of installonly packages.

    Default: ``3``.

.. _installroot_options-label:

``installroot``
    :ref:`string <string-label>`

    The root of the filesystem for all packaging operations.
    It requires an absolute path.
    See also :ref:`--installroot commandline option <installroot_options-label>`.

    Default: ``/``.

.. _install_weak_deps_options-label:

``install_weak_deps``
    :ref:`boolean <boolean-label>`

    If enabled, when a new package is about to be installed, all packages linked by weak dependency
    relation (Recommends or Supplements flags) with this package will be pulled into the transaction.

    Default: ``True``.

.. _keepcache_options-label:

``keepcache``
    :ref:`boolean <boolean-label>`

    If enabled, keeps downloaded packages in the cache. If disabled cache will persist
    until the next successful transaction even if no packages have been installed.

    Default: ``False``.

.. _logdir_options-label:

``logdir``
    :ref:`string <string-label>`

    Directory where the log files will be stored.

    Regular user default: ``~/.local/state``.

    Superuser default: ``/var/log``.

.. _log_rotate_options-label:

``log_rotate``
    :ref:`integer <integer-label>`

    Log files are rotated ``log_rotate`` times before being removed.
    If ``log_rotate`` is ``0``, the rotation is not performed.

    Default: ``4``.

.. _log_size_options-label:

``log_size``
    :ref:`storage size <storage_size-label>`

    Log  files are rotated when they grow bigger than ``log_size`` bytes. If
    ``log_size`` is ``0``, the rotation is not performed.

    The size applies for individual log files, not the sum of all log files.
    See also :ref:`log_rotate <log_rotate_options-label>`.

    Default: ``1M``.

.. _module_platform_id_options-label:

``module_platform_id``
    :ref:`string <string-label>`

    Set this to ``$name:$stream`` to override ``PLATFORM_ID`` detected from ``/etc/os-release``.
    It is necessary to perform a system upgrade and switch to a new platform.

    Default: empty.

.. _module_stream_switch_options-label:

``module_stream_switch``
    :ref:`boolean <boolean-label>`

    If enabled, allows switching enabled streams of a module.

    Default: ``False``.

.. _multilib_policy_options-label:

``multilib_policy``
    :ref:`string <string-label>`

    Controls how multilib packages are treated during install operations.

    Can either be ``best`` for the depsolver to prefer packages which best match the system's
    architecture, or ``all`` to install packages for all available architectures.

    Default: ``best``.

.. _obsoletes_options-label:

``obsoletes``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 uses obsoletes processing logic, which means it checks whether
    any dependencies of given package are no longer required and removes them.

    Useful when doing distribution level upgrades.

    It has effect during install/upgrade processes.

    Command-line option: :ref:`--obsoletes <obsoletes_options-label>`

    Default: ``True``.

.. _optional_metadata_types_options-label:

``optional_metadata_types``
    :ref:`list <list-label>`

    List of the following: ``comps``, ``filelists``, ``other``, ``presto``, ``updateinfo``, ``all``

    Specifies the types of metadata to load in addition to the essential ``primary`` and ``modules`` metadata, which are always loaded. Note that individual DNF commands may extend this list at runtime.

    Note: The list includes only metadata types recognized by DNF. However, a repository's metadata may include various other types (e.g., AppStream or metadata stored as databases instead of XML files). The special value ``all`` represents all metadata types present in the repository, including those unknown to DNF.

    Default: ``comps,updateinfo``

.. _persistdir_options-label:

``persistdir``
    :ref:`string <string-label>`

    Directory where DNF5 stores its persistent data between runs.

    Default: ``/var/lib/dnf``.

.. _pluginconfpath_options-label:

``pluginconfpath``
    :ref:`list <list-label>`

    List of directories that are searched for libdnf5 plugin configurations to load.

    All configuration files found in these directories, that are named same as a
    plugin, are parsed.

    Default: ``/etc/dnf/libdnf5-plugins``.

.. _pluginpath_options-label:

``pluginpath``
    :ref:`list <list-label>`

    List of directories that are searched for libdnf5 plugins to load. Plugins found in
    *any of the directories* in this configuration option are used.

    Default: ``/usr/lib64/libdnf5/plugins/``

.. _plugins_options-label:

``plugins``
    :ref:`boolean <boolean-label>`

    If enabled, libdnf5 plugins are enabled.

    Default: ``True``.

.. _protected_packages_options-label:

``protected_packages``
    :ref:`list <list-label>`

    This append list option contains names of packages that DNF5 should never completely remove.

    They are protected via Obsoletes as well as user/plugin removals.

    Default: ``dnf5,glob:/etc/dnf/protected.d/*.conf``.

    .. NOTE::
       Any packages which should be protected can do so by including a file in ``/etc/dnf/protected.d``
       with their  package name in it.

       DNF5 will protect also the package corresponding to the running version of the kernel. See also
       :ref:`protect_running_kernel <protect_running_kernel_options-label>` option.

.. _protect_running_kernel_options-label:

``protect_running_kernel``
    :ref:`boolean <boolean-label>`

    Controls whether the package corresponding to the running version of kernel is protected from removal.

    Default: ``True``.

    .. NOTE::
       YUM compatibility option

.. _recent_options-label:

``recent``
    :ref:`integer <integer-label>`

    Sets the time period in days used for the ``--recent`` option in the :ref:`repoquery <repoquery_command_ref-label>`,
    :ref:`info <info_command_ref-label>` and :ref:`list <list_command_ref-label>` commands.

    Default: 7

.. _reposdir_options-label:

``reposdir``
    :ref:`list <list-label>`

    Repository configuration files locations.

    The behavior of ``reposdir`` could differ when it is used
    along with \-\ :ref:`-installroot <installroot_options-label>` option.

    Default: [``/etc/yum.repos.d``, ``/etc/distro.repos.d``, ``/usr/share/dnf5/repos.d``]

.. _skip_broken_options-label:

``skip_broken``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will skip uninstallable packages instead of failing while
    resolving dependencies.

    Default: ``False``.

.. _skip_unavailable_options-label:

``skip_unavailable``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will skip unavailable packages instead of failing while
    preparing rpm transactions.

    Default: ``False``.

.. _system_cachedir_options-label:

``system_cachedir``

    :ref:`string <string-label>`

    For superuser overwrites the :ref:`cachedir <cachedir_options-label>` option value.

    Allows one to differentiate user and superuser cachedir.

    Default: ``/var/cache/libdnf5``.

.. _system_state_dir_options-label:

``system_state_dir``
    :ref:`string <string-label>`

    System state files location. See :manpage:`dnf5-system-state(7)`, :ref:`system state <systemstate_misc_ref-label>` for details.

    Default: ``/usr/lib/sysimage/libdnf5``.

.. _transaction_history_dir_options-label:

``transaction_history_dir``

    :ref:`string <string-label>`

    History database location.

    By default it has the same value as :ref:`system_state_dir <_system_state_dir_options-label>`.

.. _tsflags_options-label:

``tsflags``
    :ref:`list <list-label>`

    List of strings adding extra flags for the RPM transaction.

    ================  ===============================
    tsflag value      RPM Transaction Flag
    ================  ===============================
    ``noscripts``     ``RPMTRANS_FLAG_NOSCRIPTS``
    ``test``          ``RPMTRANS_FLAG_TEST``
    ``notriggers``    ``RPMTRANS_FLAG_NOTRIGGERS``
    ``nodocs``        ``RPMTRANS_FLAG_NODOCS``
    ``justdb``        ``RPMTRANS_FLAG_JUSTDB``
    ``nocontexts``    ``RPMTRANS_FLAG_NOCONTEXTS``
    ``nocaps``        ``RPMTRANS_FLAG_NOCAPS``
    ``nocrypto``      ``RPMTRANS_FLAG_NOFILEDIGEST``
    ``deploops``      ``RPMTRANS_FLAG_DEPLOOPS``
    ================  ===============================

    The ``nocrypto`` option will also set the ``_RPMVSF_NOSIGNATURES`` and
    ``_RPMVSF_NODIGESTS`` VS flags.

    The ``test`` option provides a transaction check without performing the transaction.
    It includes downloading of packages, OpenPGP keys check (including permanent import of
    additional keys if necessary), and rpm check to prevent file conflicts.

    The ``nocaps`` is supported with rpm-4.14 or later. When ``nocaps`` is used but rpm
    doesn't support it, DNF5 only reports it as an invalid tsflag.

    Default: empty.

.. _use_host_config_options-label:

``use_host_config``

    Use configuration files and variable definitions from the host system rather
    than the installroot.
    :ref:`See <installroot_misc_ref-label>` :manpage:`dnf5-installroot(7)` for more info.

    Default: ``False``.

.. _varsdir_options-label:

``varsdir``
    :ref:`list <list-label>`

    List of directories where variables definition files are looked for.

    See :ref:`variable files <varfiles-label>` in Configuration reference.


    Default: ``/etc/dnf/vars``.

.. _zchunk_options-label:

``zchunk``
    :ref:`boolean <boolean-label>`

    If enabled, repository metadata are compressed using the zchunk format (if available).

    Default: ``True``.

[main] Options - Colors
=======================

.. _color_list_available_upgrade_options-label:

``color_list_available_upgrade``
    :ref:`color <color-label>`

    Color of available packages that are newer than installed packages.
    The option is used during list operations.

    Default: ``bold,blue``.

.. _color_list_available_downgrade_options-label:

``color_list_available_downgrade``
    :ref:`color <color-label>`

    Color of available packages that are older than installed packages.
    The option is used during list operations.

    Default: ``dim,magenta``.

.. _color_list_available_reinstall_options-label:

``color_list_available_reinstall``
    :ref:`color <color-label>`

    Color of available packages that are identical to installed versions and are available for reinstalls.
    The option is used during list operations.

    Default: ``bold,green``.

.. _color_list_available_install_options-label:

``color_list_available_install``
    :ref:`color <color-label>`

    Color of packages that are available for installation and none of their versions in installed.
    The option is used during list operations.

    Default: ``bold,cyan``.

.. _color_update_installed_options-label:

``color_update_installed``
    :ref:`color <color-label>`

    Color of removed packages.
    This option is used during displaying transactions.

    Default: ``dim,red``.

.. _color_update_local_options-label:

``color_update_label``
    :ref:`color <color-label>`

    Color of local packages that are installed from the @commandline repository.
    This option is used during displaying transactions.

    Default: ``dim,green``.

.. _color_update_remote_options-label:

``color_update_remote``
    :ref:`color <color-label>`

    Color of packages that are installed/upgraded/downgraded from remote repositories.
    This option is used during displaying transactions.

    Default: ``bold,green``.

.. _color_search_match_options-label:

``color_search_match``
    :ref:`color <color-label>`

    Color of patterns matched in search output.

    Default: ``bold,magenta``.


Repo Options
============

.. _baseurl_repo_options-label:

``baseurl``
    :ref:`list <list-label>`

    List of URLs for the repository.

    Default [].

    URLs are tried in the listed order (equivalent to yum’s “failovermethod=priority” behaviour).

.. _cost_repo_options-label:

``cost``
    :ref:`integer <integer-label>`

    The relative cost of accessing this repository, defaulting to 1000. This
    value is compared when the priorities of two repositories are the same. The
    repository with the lowest cost is picked. It is useful to make the library
    prefer on-disk repositories to remote ones.

.. _enabled_repo_options-label:

``enabled``
    :ref:`boolean <boolean-label>`

    Include this repository as a package source.

    Default: ``True``.

.. _gpgkey_repo_options-label:

``gpgkey``
    :ref:`list <list-label>`

    URLs of a OpenPGP key files that can be used for signing metadata and packages
    of this repository. If a file can not be verified using
    the already imported keys, import of keys from this option is attempted and
    the keys are then used for verification.

    Default: ``[]``

.. _metalink_repo_options-label:

``metalink``
    :ref:`string <string-label>`

    URL of a metalink for the repository.

    Default: ``None``.

.. _mirrorlist_repo_options-label:

``mirrorlist``
    :ref:`string <string-label>`

    URL of a mirrorlist for the repository.

    Default: ``None``.


.. _name_repo_options-label:

``name``
    :ref:`string <string-label>`

    A human-readable name of the repository. Defaults to the ID of the repository.

.. _priority_repo_options-label:

``priority``
    :ref:`integer <integer-label>`

    The priority value of this repository. If there is more than
    one candidate package for a particular operation, the one from a repo with
    the lowest priority value is picked, possibly despite being less convenient
    otherwise (e.g. by being a lower version).

    Default: 99

.. _type_repo_options-label:

``type``
    :ref:`string <string-label>`

    Type of repository metadata. Supported values are: ``rpm-md``. Aliases for
    ``rpm-md``: ``rpm``, ``repomd``, ``rpmmd``, ``yum``, ``YUM``.

    Default: empty.


Source and debuginfo repository names
=====================================

For a given repository with an identifier in the form "<ID>-rpms", its corresponding source repository is expected to have an identifier in the form "<ID>-source-rpms" and debuginfo repository an identifier in the form "<ID>-debug-rpms". Otherwise (if the repository identifier doesn't have the "-rpms" suffix), the source repository is expected to have an identifier in the form "<ID>-source" and debuginfo repository an identifier in the form "<ID>-debuginfo".

For example, for repository "fedora", the source repository is "fedora-source" and debuginfo repository is "fedora-debuginfo". For repository "fedora-rpms", the source repository is "fedora-source-rpms" and debuginfo repository is "fedora-debug-rpms".

.. _repo_variables-label:

Repo Variables
==============

Right side of every repo option can be enriched by the following variables:

``$arch``

    Refers to the system’s CPU architecture e.g, aarch64, i586, i686 and x86_64.

``$basearch``

    Refers to the base architecture of the system. For example, i686 and i586 machines
    both have a base architecture of i386, and AMD64 and Intel64 machines have a base architecture of x86_64.

``$releasever``

    Refers to the release version of operating system which DNF5 derives from information available in RPMDB.


In addition to these hard coded variables, user-defined ones can also be used.
They can be defined either via :ref:`variable files <varfiles-label>`, or by using special environmental variables.
The names of these variables must be prefixed with DNF_VAR\_ and they can only consist of alphanumeric characters
and underscores::

    $ DNF_VAR_MY_VARIABLE=value

To use such variable in your repository configuration remove the prefix. E.g.::

    [myrepo]
    baseurl=https://example.site/pub/fedora/$MY_VARIABLE/releases/$releasever

Note that it is not possible to override the ``arch`` and ``basearch`` variables using either variable files or
environmental variables.

Although users are encouraged to use named variables, the numbered environmental variables
``DNF0`` - ``DNF9`` are still supported::

    $ DNF1=value

    [myrepo]
    baseurl=https://example.site/pub/fedora/$DNF1/releases/$releasever

Options for both [main] and Repo
================================

Some options can be applied in either the main section, per repository, or in a
combination. The value provided in the main section is used for all repositories
as the default value, which repositories can then override in their
configuration.


.. _bandwidth_options-label:

``bandwidth``
    :ref:`storage size <storage_size-label>`

    Total bandwidth available for downloading.
    Meaningful when used with the :ref:`throttle option <throttle_options-label>`.

    Default: ``0``.

.. _build_cache_options-label:

``build_cache``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will save libsolv cache generated from downloaded metadata
    to cachedir. These solv files are loaded during subsequent runs which
    significantly speeds up DNF5.

    Default: ``True``.

.. _countme_options-label:

``countme``
    :ref:`boolean <boolean-label>`

    When enabled, one (and only one) HTTP GET request for the metalink file
    will be selected at random every week to carry a special URL flag.

    This flag allows the repository provider to estimate the number of systems
    consuming the repository, by counting such requests over a week's time.
    This method is more accurate than just counting unique IP addresses (which
    is subject to both overcounting and undercounting due to short DHCP leases
    and NAT, respectively).

    This is *not* an out-of-band HTTP request made for this purpose alone.
    Only requests initiated by DNF during normal operation, such as to check
    for metadata updates, can get this flag.

    The flag is a simple "countme=N" parameter appended to the metalink URL
    where N is an integer representing the age "bucket" this system belongs to.
    Four buckets are defined, based on how many full weeks have passed since
    the installation of a system:

    ======  ===============================
    bucket  system age
    ======  ===============================
    1       first week
    2       first month (2 - 4 weeks)
    3       first 6 months (5 - 24 weeks)
    4       more than 6 months (> 24 weeks)
    ======  ===============================

    This number is meant to help distinguish short-lived (throwaway) machines
    from long-term installs and get a better picture of how systems are used
    over time.

    To determine a system's installation time ("epoch"), the ``machine-id(5)``
    file's modification time is used as the single source of truth. This file
    is semantically tied to the system's lifetime as it's typically populated
    at installation time or during the first boot by an installer tool or init
    system (such as ``systemd(1)``), respectively, and remains unchanged.

    If the file is empty or missing (such as in containers), the time of the
    very first request made using the expanded metalink URL (i.e. with any
    repository variables such as ``$releasever`` substituted) that carried the
    flag is declared as the epoch.

    If no metalink URL is defined for this repository but a mirrorlist URL is,
    the latter is used for this purpose instead.

    Default: ``False``.

.. _disable_excludes_options-label:
``disable_excludes``
    :ref:`list <list-label>`

    Used to disable package, group and environment filtering. It can include a list of repository IDs (globs allowed) for which repository-specific package excludes are ignored. If it contains the special value ``main``, excludes from the main configuration files are bypassed. If it contains the special value ``*``, no excludes filtering is applied at all.

.. _enablegroups_options-label:

``enablegroups``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will allow the use of package groups.

    Default: ``True``.

.. _excludepkgs_options-label:

``excludepkgs``
    :ref:`list <list-label>`

    Exclude packages of this repository, specified by a name or a glob and
    separated by a comma, from all operations.

    Can be disabled using ``disable_excludes`` config option.

    Default: ``[]``.

.. _fastestmirror_options-label:

``fastestmirror``
    :ref:`boolean <boolean-label>`

    If enabled, TCP socket latency is used to find the closest available mirror.
    A mirror is then selected at random with less than twice the lowest latency for load balancing purposes.
    This overrides the order provided by the mirrorlist/metalink file itself,
    and does not take into consideration mirrorlist parameters such as mirror bandwidth nor preferred mirrors for client IP addresses.

    Default: ``False``.

.. _pkg_gpgcheck_options-label:
.. _gpgcheck_options-label:

``pkg_gpgcheck``
    :ref:`boolean <boolean-label>`

    Whether to perform OpenPGP signature check on packages found in this repository.

    The default is False.

    Doesn't apply for packages passed directly as arguments, as they are not in any repository,
    see :ref:`localpkg_gpgcheck <localpkg_gpgcheck_options-label>`.

    Due to compatibility `gpgcheck` option is supported as well but `pkg_gpgcheck` is preferred.

.. _includepkgs_options-label:

``includepkgs``
    :ref:`list <list-label>`

    Include packages of this repository, specified by a name or a glob and separated by a comma, in all operations.

    Inverse of :ref:`excludepkgs <excludepkgs_options-label>`, DNF5 will exclude any package in the repository
    that doesn't match this list.

    This works in conjunction with :ref:`excludepkgs <excludepkgs_options-label>` and doesn't override it,
    so if you 'excludepkgs=*.i386' and 'includepkgs=python*' then only packages starting with python
    that do not have an i386 arch will be seen by DNF5 in this repo.

    Can be disabled using ``disable_excludes`` config option.

    Default: ``[]``.

.. _ip_resolve_options-label:

``ip_resolve``
    :ref:`ip address <ip_address_type-label>`

    Determines how DNF5 resolves host names. Set this to ``4``, ``IPv4``, ``6``, ``IPv6``
    to resolve to IPv4 or IPv6 addresses only.

    Default: ``whatever``.

.. _localpkg_gpgcheck_options-label:

``localpkg_gpgcheck``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will perform a OpenPGP signature check on local packages (packages in a file, not in a repository).

    Default: ``False``.

.. _max_parallel_downloads_options-label:

``max_parallel_downloads``
    :ref:`integer <integer-label>`

    Maximum number of simultaneous downloads. Max is ``20``.

    Default: ``3``.

.. _metadata_expire_options-label:

``metadata_expire``
    :ref:`time in seconds <time_in_seconds-label>`

    The period after which the remote repository is checked for metadata update and in the positive
    case the local metadata cache is updated.
    It can be ``-1`` or ``never`` to make the repo never considered expired.

    Expire of metadata can be also triggered by change of timestamp of configuration files
    (``dnf.conf``, ``<repo>.repo``).

    See also :ref:`check_config_file_age <check_config_file_age_options-label>`.

    Default: ``60 * 60 * 48``, 48 hours.

.. _minrate_options-label:

``minrate``
    :ref:`storage size <storage_size-label>`

    Sets the low speed threshold in bytes per second.
    If the server is sending data at the same or slower speed than this value for at least
    :ref:`timeout option <timeout_options-label>` seconds, DNF5 aborts the connection.

    Default: ``1000``.

.. _password_options-label:

``password``
    :ref:`string <string-label>`

    The password used to connect to a repository with basic HTTP authentication.

    Default: empty.

.. _proxy_options-label:

``proxy``
    :ref:`string <string-label>`

    URL of a proxy server to connect through.

    Set to an empty string in the repository configuration to disable proxy
    setting inherited from the main section. The expected format of this option is
    ``<scheme>://<ip-or-hostname>[:port]``.
    (For backward compatibility, '_none_' can be used instead of the empty string.)

    Default: empty.

    .. NOTE::
       The curl environment variables (such as ``http_proxy``) are effective if this option is unset
       (or '_none_' is set in the repository configuration). See the ``curl`` man page for details.

.. _proxy_username_options-label:

``proxy_username``
    :ref:`string <string-label>`

    The username to use for connecting to the proxy server.

    Default: empty.

.. _proxy_password_options-label:

``proxy_password``
    :ref:`string <string-label>`

    The password to use for connecting to the proxy server.

    Default: empty.

.. _proxy_auth_method_options-label:

``proxy_auth_method``
    :ref:`string <string-label>`

    The authentication method used by the proxy server. Valid values are

    ==============     ==========================================================
    method             meaning
    ==============     ==========================================================
    ``basic``          HTTP Basic authentication
    ``digest``         HTTP Digest authentication
    ``negotiate``      HTTP Negotiate (SPNEGO) authentication
    ``ntlm``           HTTP NTLM authentication
    ``digest_ie``      HTTP Digest authentication with an IE flavor
    ``ntlm_wb``        NTLM delegating to winbind helper
    ``none``           None auth method
    ``any``            All suitable methods
    ==============     ==========================================================

    Default: ``any``.

.. _proxy_sslcacert_options-label:

``proxy_sslcacert``
    :ref:`string <string-label>`

    Path to the file containing the certificate authorities to verify proxy SSL certificates.

    Default: empty, uses system default.

.. _proxy_sslclientcert_options-label:

``proxy_sslclientcert``
    :ref:`string <string-label>`

    Path to the SSL client certificate used to connect to proxy server.

    Default: empty.

.. _proxy_sslclientkey_options-label:

``proxy_sslclientkey``
    :ref:`string <string-label>`

    Path to the SSL client key used to connect to proxy server.

    Default: empty.

.. _proxy_sslverify_options-label:

``proxy_sslverify``
    :ref:`boolean <boolean-label>`

    If enabled, proxy SSL certificates are verified. If the client can not be authenticated, connecting fails and the
    repository is not used any further. If ``False``, SSL connections can be used, but certificates are not verified.

    Default: ``True``.

.. _repo_gpgcheck_options-label:

``repo_gpgcheck``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will perform OpenPGP signature check on this repository's metadata.


    .. NOTE::
       OpenPGP keys for this check are stored separately from OpenPGP keys used in package signature
       verification. Furthermore, they are also stored separately for each repository.

       This means that DNF5 may ask to import the same key multiple times. For example, when a key was
       already imported for package signature verification and this option is turned on, it may be needed
       to import it again for the repository.

    Default: ``False``.

.. _skip_if_unavailable_options-label:

``skip_if_unavailable``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will continue running and disable the repository that couldn't be synchronized
    for any reason. This option doesn't affect skipping of unavailable packages after dependency
    resolution. To check inaccessibility of repository use it in combination with
    :ref:`refresh command line option <refresh_command_options-label>`.

    Default: ``False``.

    .. NOTE::
       this option in particular :ref:`can be set in your configuration file
       by your distribution <conf_distribution_specific_options-label>`.

.. _sslcacert_options-label:

``sslcacert``
    :ref:`string <string-label>`

    Path to the file containing the certificate authorities to verify SSL certificates.

    Default: empty, uses system default.

.. _sslclientcert_options-label:

``sslclientcert``
    :ref:`string <string-label>`

    Path to the SSL client certificate used to connect to remote sites.

    Default: empty.

.. _sslclientkey_options-label:

``sslclientkey``
    :ref:`string <string-label>`

    Path to the SSL client key used to connect to remote sites.

    Default: empty.

.. _sslverify_options-label:

``sslverify``
    :ref:`boolean <boolean-label>`

    If enabled, remote SSL certificates are verified. If the client can not be authenticated,
    connecting fails and the repository is not used any further.
    If disabled, SSL connections can be used, but certificates are not verified.

    Default: ``True``.

.. _throttle_options-label:

``throttle``
    :ref:`storage size <storage_size-label>`

    Limits the downloading speed. It might be an absolute value or a percentage, relative to the value of the
    :ref:`bandwidth option <bandwidth_options-label>` option. ``0`` means no throttling.

    Default: ``0``.

.. _timeout_options-label:

``timeout``
    :ref:`time in seconds <time_in_seconds-label>`

    Number of seconds to wait for a connection before timing out. Used in combination with
    :ref:`minrate option <minrate_options-label>` option.

    Default: ``30``.

.. _username_options-label:

``username``
    :ref:`string <string-label>`

    The username to use for connecting to repo with basic HTTP authentication.

    Default: empty.

.. _user_agent_options-label:

``user_agent``
    :ref:`string <string-label>`

    The User-Agent string to include in HTTP requests sent by DNF5.

    Default: ::

        libdnf (NAME VERSION_ID; VARIANT_ID; OS.BASEARCH)

    .. NOTE::
       ``NAME``, ``VERSION_ID`` and ``VARIANT_ID`` are OS identifiers read from the
       :manpage:`os-release(5)` file, and ``OS`` and ``BASEARCH`` are the canonical OS
       name and base architecture, respectively.
       Example: ::

           libdnf (Fedora 39; server; Linux.x86_64)


Types of Options
================

.. _boolean-label:

``boolean``
    Data type with only two possible values.

    One of following options can be used: ``1``, ``0``, ``True``, ``False``, ``yes``, ``no``.

.. _color-label:

``color``
    String describing color and modifiers separated with a comma, for example ``red,bold``.

    * Colors: ``black``, ``blue``, ``cyan``, ``green``, ``magenta``, ``red``, ``white``, ``yellow``.
    * Modifiers: ``bold``, ``blink``, ``dim``, ``normal``, ``reverse``, ``underline``.

.. _integer-label:

``integer``
    Whole number that can be written without a fractional component.

.. _ip_address_type-label:

``ip address type``
    String describing ip address types.

    One of the following options can be used: ``4``, ``IPv4``, ``6``, ``IPv6``.

.. _list-label:

``list``
    String representing one or more strings separated by space or comma characters.

.. _storage_size-label:

``storage size``
    String representing storage sizes formed by an integer and a unit.

    Valid units are ``k``, ``M``, ``G``.

.. _string-label:

``string``
    It is a sequence of symbols or digits without any whitespace character.

.. _time_in_seconds-label:

``time in seconds``
    String representing time units in seconds. Can be set to ``-1`` or ``never``.


.. _drop_in_configuration_directories-label:

Drop-in configuration directories
=================================

`DNF5` loads configuration options that are defined in the :ref:`main
configuration file <main_configuration_file-label>`, :ref:`user configuration
files<user_configuration_files-label>` and :ref:`distribution configuration
files<distro_configuration_files-label>`.

The configuration files are processed following this order:

1. Configuration files are alphabetically sorted in a list of names from the
   :ref:`distribution configuration directory<distro_configuration_dir-label>.
   and the :ref:`user configuration directory<user_configuration_dir-label>`. If
   a file with the same name is present in both directories, only the file from
   the user configuration directory is added to the list. The
   distribution file is then masked by the user file.
2. Options are retrieved in order from the list The configuration from the next
   file overrides the previous one. The last option wins.

Example of configuration files
------------------------------

User configuration files:

- /etc/dnf/dnf.conf
- /etc/dnf/libdnf5.conf.d/20-user-settings.conf
- /etc/dnf/libdnf5.conf.d/60-something.conf
- /etc/dnf/libdnf5.conf.d/80-user-settings.conf

Distribution configuration files:

- /usr/share/dnf5/libdnf.conf.d/50-something.conf
- /usr/share/dnf5/libdnf.conf.d/60-something.conf
- /usr/share/dnf5/libdnf.conf.d/90-something.conf

Resulting file loading order by default
(/usr/share/dnf5/libdnf.conf.d/60-something.conf is skipped, masked by
the user file /etc/dnf/libdnf5.conf.d/60-something.conf):

1. /etc/dnf/libdnf5.conf.d/20-user-settings.conf
2. /usr/share/dnf5/libdnf.conf.d/50-something.conf
3. /etc/dnf/libdnf5.conf.d/60-something.conf
4. /etc/dnf/libdnf5.conf.d/80-user-settings.conf
5. /usr/share/dnf5/libdnf.conf.d/90-something.conf
6. /etc/dnf/dnf.conf

.. _conf_files_and_directories-label:

Files and directories
---------------------

.. _user_configuration_dir-label:

``User Configuration Directory``
    /etc/dnf/libdnf5.conf.d/

.. _user_configuration_files-label:

``User Configuration Files``
    /etc/dnf/libdnf5.conf.d/20-user-settings.conf

.. _distro_configuration_dir-label:

``Distribution Configuration Directory``
    /usr/share/dnf5/libdnf.conf.d/

.. _distro_configuration_files-label:

``Distribution Configuration Files``
    /usr/share/dnf5/libdnf.conf.d/50-something.conf


.. _drop_in_repo_directories-label:

Drop-in repo directories
========================

After the repository configurations are loaded other repo configurations can be overloaded from the directories
:ref:`user repos override directory <user_repos_override_dir-label>`
and :ref:`distribution repos override directory <distro_repos_override_dir-label>`.

The format of the files inside the directories is the same as the format of the repository configuration files.
The options in the overridden files can modify existing repos but cannot create new repositories.

Override files support globs in the repository ID in order to support bulk modifications of repository parameters.

The repository overrides are processed following this order:

1. Files from ``/usr/share/dnf5/repos.override.d/`` and ``/etc/dnf5/repos.override.d/`` are loaded in an alphabetically
   sorted list. In case files have the same name, the file from ``/etc/dnf5/repos.override.d/`` is used.
   This implies the list has only unique filenames. This also implies that the repository configuration files can be
   simply masked by creating a file with the same name in the ``/etc`` override directory.

2. The options from the files are applied in the order they are loaded. The last option wins.


.. note::
    See also the documentation for :ref:`Config-manager Command<config_manager_plugin_ref-label>` to see where the overrides are used.

Example configuration
---------------------

.. code-block::

   # Enable `skip_if_unavailable` for all repositories
   [*]
   skip_if_unavailable = true

   # And then disable `skip_if_unavailable` for repositories with id prefix "fedora"
   [fedora*]
   skip_if_unavailable = false

Example of configuration files
------------------------------

This example shows the order in which override files are processed.

Files with user repos overrides:

- /etc/dnf/repos.overide.d/20-user-overrides.repo
- /etc/dnf/repos.overide.d/60-something2.repo
- /etc/dnf/repos.overide.d/80-user-overrides.repo
- /etc/dnf/repos.overide.d/99-config-manager.repo

Files with distribution repos overrides:

- /usr/share/dnf5/repos.overide.d/50-something2.repo
- /usr/share/dnf5/repos.overide.d/60-something2.repo
- /usr/share/dnf5/repos.overide.d/90-something2.repo

Resulting file processing order:

1. /etc/dnf/repos.overide.d/20-user-overrides.repo
2. /usr/share/dnf5/repos.overide.d/50-something2.repo
3. /etc/dnf/repos.overide.d/60-something2.repo
4. /etc/dnf/repos.overide.d/80-user-overrides.repo
5. /usr/share/dnf5/repos.overide.d/90-something2.repo
6. /etc/dnf/repos.overide.d/99-config-manager.repo


.. _repo_files_and_directories-label:

Files and directories
---------------------

.. _user_repos_override_dir-label:

``User Repos Override Directory``
    /etc/dnf/repos.override.d/

.. _user_repos_override_files-label:

``User Repos Override Files``
    /etc/dnf/repos.override.d/20-user-overrides.repo

.. _distro_repos_override_dir-label:

``Distribution Repos Override Directory``
    /usr/share/dnf5/repos.override.d/

.. _distro_repos_override_files-label:

``Distribution Repos Override Files``
    /usr/share/dnf5/repos.override.d/50-something2.repo

Files
=====

.. _main_configuration_file-label:

``Main Configuration File``
    /etc/dnf/dnf.conf

.. _cache_files-label:

``Cache Files``
    /var/cache/libdnf5

.. _repo_files-label:

``Repository Files``
    /etc/yum.repos.d/

.. _varfiles-label:

``Variables``
    Any property named file in ``/etc/dnf/vars`` is turned into a variable named after the filename
    (or overrides any of the above variables but those set from commandline).
    Filenames may contain only alphanumeric characters and underscores and be in lowercase.
    Variables are also read from ``/etc/yum/vars`` for YUM compatibility reasons.


Directories
===========

Repository Directories
----------------------

Directories are listed in the order in which they are processed.

.. note::
   Unlike overrides that modify an existing repository configuration,
   the configuration in these directories defines new repositories.
   It is not possible to define repositories with the same ID.
   If multiple repositories with the same ID are defined only the first configuration
   will be loaded. Loading the next one will log an error.

.. _yum_repos_dir-label:

``YUM Repositories Directory``
    /etc/yum.repos.d/

.. _distro_repos_dir-label:

``Distribution Repositories Directory in /etc``
    /etc/distro.repos.d/

``Distribution Repositories Directory in /usr``
    /usr/share/dnf5/repos.d/

Variables Directories
---------------------

.. _vars_dir-label:

``Variables Directory``
    /etc/dnf/vars/

.. _distro_vars_dir-label:

``Distribution Variables Directory``
    /usr/share/dnf5/vars.d/

See Also
========

* :manpage:`dnf5(8)`, :ref:`DNF5 Command Reference <command_ref-label>`
* :manpage:`dnf5.conf-todo(5)`, :ref:`Options that are documented/implemented in DNF but not in DNF5 <dnf5_conf_todo-label>`
* :manpage:`dnf5.conf-deprecated(5)`, :ref:`Config Options that are deprecated in DNF5 <dnf5_conf_deprecated-label>`
