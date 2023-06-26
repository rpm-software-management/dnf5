====================================
Changes in DNF5 in comparison to DNF
====================================

The chapter describe differences between `DNF5` (https://github.com/rpm-software-management/dnf5) and `DNF`
(https://github.com/rpm-software-management/dnf)

Changes on the API:
===================
PackageSet::operator[]
----------------------
It was removed due to insufficient O(n^2) performance.
Use PackageSet iterator to access the data instead.


Package::get_epoch()
--------------------
The return type was changed from `unsigned long` to `std::string`.


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
Combine `query.filter_priority()` with `query.filter_latest_evr()` or another filter to achieve the original
functionality.


Query::filter() - HY_PKG_LATEST
-------------------------------
The filter was replaced with `filter_latest_evr()` which has the same behavior as `HY_PKG_LATEST_PER_ARCH`


ConfigMain::proxy_auth_method() and ConfigRepo::proxy_auth_method()
-------------------------------------------------------------------
The return types were changed. `OptionEnum<std::string>` was replaced by `OptionStringSet`.
A combination of several authentication methods (for example "basic" and "digest") can now be used.
This allows using a list of authentication methods in configuration files and the DNF5 command line
"--setopt=proxy_auth_method=".


Changes on the command line:
============================

Commands cannot have optional subcommands and optional arguments. Is some cases subcommand can have the same string as
an argument. It means there is a difficulty to find advisory for package with the name `list`, `info`, or `summary`.
`dnf history info <transaction ID>` -> `dnf history --info <transaction ID>`
`dnf updateinfo info` -> `dnf updateinfo --info`

Options that cannot be applied to all command or can be applied but without any effect should be removed from general
options and implemented only for related commands
`--best`, `--nobest` are only related several transaction commands

Renaming boolean options to format `--<option>`, and `--no-<option>`
`--nobest` -> `--no-best`. Proposing to keep `--nobest` as a deprecated option.

To disable all configuration file excludes, the `*` glob character is used now instead of the `all` within
the `disable_excludes` configuration option to unify the behavior with query objects on the API.

strict configuration option deprecation
---------------------------------------
`strict` config option is now deprecated. The problem with this option is that it does two things:

 1. if disabled it allows the solver to skip uninstallable packages to resolve depsolv problems
 2. if disabled it allows dnf to skip unavailable packages (this is for `install` command mostly)

The functionality is now split to two config options - `skip_broken` for the uninstallable packages and
`skip_unavailable` for packages not present in repositories. Together with these new config options there are also
corresponding command line options `--skip-broken` and `--skip-unavailable` for commands where it makes sense.


Autoremove command
------------------
 * Dropped `<spec>` positional argument since the usecase is sufficiently covered by the `remove` command.
 * Specific `autoremove-n`, `autoremove-na`, and `autoremove-nevra` variants of the command are not supported.

Distro-sync command
-------------------
 * When any argument does not match any package or it is not installed, DNF5 fail. The behavior can be modified by
   the `--skip-unavailable` option.

Downgrade command
-----------------
 * When any argument does not match any package or it is not installed, DNF5 fail. The behavior can be modified by
   the `--skip-unavailable` option.

Group command
-------------
 * Dropped `group mark install` and `group mark remove` subcommands in favour of the
   new `--no-packages` option of the `group install/remove` commands. So for example
   to mark a group as installed without touching any packages,
   `dnf5 group install --no-packages <group_id>` command can be used.

List command
------------
 * Dropped `--all` option since this behavior is now the default one.
 * Changed the list of `--available` packages. Previously, dnf4 only listed packages that are either not installed, or
   whose version is higher than the installed version. Now this behaviour is kept when no modifier is used - to skip
   packages already listed in the `Installed Packages` section to reduce duplicities. But if the `--available` modifier
   is used, dnf5 considers all versions available in the enabled repositories, regardless of which version is installed.

Remove command
--------------
 * Dropped `--noautoremove` option. The behavior for automatic removing of dependencies is now controlled by the
   `clean_requirements_on_remove` configuration option which is set to `True` by default.

Repoclosure command
-------------------
 * Dropped `--pkg`` option. Positional arguments can be used to specify packages to check closure for.

Repoquery command
-----------------
 * Dropped: `-a/--all`, `--alldeps`, `--nevra` options, their behavior is and has been the default for both dnf4 and
   dnf5. The options are no longer needed.
 * Dropped: `--nvr`, `--envra` options. They are no longer supported.
 * Dropped: `--archlist` alias for `--arch`.
 * Dropped: `-f` alias for `--file` also the arguments to `--file` are separated by comma instead of a space.
 * Moved `--groupmember` option to the Group info and list commands and renamed to `--contains-pkgs`.
 * --queryformat/--qf no longer prints additional new line at the end of each formatted string, bringing it closer to
   rpm --query behavior.
 * --queryformat no longer supports `size` tag because it was printing install size for installed packages and download
   size for not-installed packages. This could be confusing.
 * Option `--list` which lists all files contained in packages was renamed to more appropriate `--files`.
 * Option `--source` was renamed to `--sourcerpm` and it now matches queryformat's `sourcerpm` tag.

Upgrade command
---------------
 * New dnf5 option `--minimal` (`upgrade-minimal` command still exists as a compatibility alias for
   `upgrade --minimal`).
 * When any argument does not match any package or it is not installed, DNF5 fail. The behavior can be modified by
   the `--skip-unavailable` option.
 * Dropped upgrade command aliases `upgrade-to` and `localupdate`.

Changes of configuration:
=========================

Default of `best` configuration option changed to `true`
--------------------------------------------------------
The new default value ensures that important updates will not be skipped and issues in distribution will be reported
earlier.
