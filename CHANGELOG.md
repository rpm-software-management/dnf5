# 5.1.6

- Update translations from weblate
- system-upgrade: Add descriptions for aliases
- system-upgrade: backwards-compat offline-{upgrade,distrosync}
- bash-completion: Prepare ownerships for dnf5 switch
- bash-completion: Complete dnf command if we own it
- dnf5: Print command line hints after resolve failure
- doc: Example of Advisory.list() API usage
- dnfdaemon: Add NEVRA field to advisory packages
- doc: Improve version parameter docs
- doc: Sync documentation in help and man pages
- doc: Review and fix missing commands
- system-upgrade: reboot: don't fail if D-Bus not available
- system-upgrade: download: use more of main.cpp's logic
- system-upgrade: clean command now deletes /system-update
- system-upgrade: offline _execute should exit cleanly
- system-upgrade: output tweaks for tests
- system-upgrade: fix --no-downgrade
- doc: Add Advisory interface to daemon documentation
- system-upgrade: fix pkgconfig systemd variable
- system-upgrade: misc fixes
- system-upgrade: Documentation
- system-upgrade: systemd behind build flag
- system-upgrade: offline status subcommand
- system-upgrade: fix DNF 4 regressions
- system-upgrade: offline-distrosync, offline-upgrade aliases
- system-upgrade: Fix installroot
- system-upgrade: Fixes and cleanup, respond to review
- system-upgrade: Handle _execute errors
- system-upgrade: Add OFFLINE_STARTED_ID comment, cleanup
- system-upgrade: Add warning to `dnf5 offline _execute`
- system-upgrade: Add `--offline` flag
- system-upgrade: Add `offline`, `system-upgrade` commands
- Workaround slow scols_table_print_range() for pkg info formatting
- dnf5: Remove redundant arch filters
- PackageQuery: Source rpms do not upgrade binary ones
- Don't set obsoletes < 5 for ELN/RHEL yet
- dnf5-plugin automatic: random_wait: Fix types
- dnf5-plugin automatic: Fix missing virtual destructor in dnf5::Emmiter
- dnf5daemon-server: Fix variables type in SessionManager::open_session
- libdnf5-cli: Fix add_line_into_provides_table func declaration

# 5.1.15

- Update translations from weblate
- Automatically set `upgrade --downloadonly` when `--destdir` is used
- Write warnings to stderr too in config-manager plugin
- Add repoid to generated repository name in config-manager plugin
- Bump sdbus-cpp requirement to 0.9.0
- Document and implement dnf5daemon Rpm interface
- Document and implement dnf5daemon Goal interface
- Document and implement dnf5daemon Repo interface
- Document and implement dnf5daemon Base interface
- Document and implement dnf5daemon Advisory interface
- Document and implement dnf5daemon SessionManager interface
- Add `dnf5daemon repo --enable/--disable` commands
- automatic: Skip network availability check without remote repo
- dnf5daemon: Rpm.list() works with commandline pkgs

# 5.1.14

- Update translations from weblate
- Make the error to resolve module metadata more descriptive
- Switch off deltarpm support
- Limit number of dnf5daemon simultaneously active sessions
- Make info and list commands case insesitive
- Allow dnf5daemon configuration overrides for root
- Add repoquery.hpp for swig-4.2.1 support

# 5.1.13

- Update translations from weblate
- build: Adapt to changes in Fedora packaging of bash-completion
- Change location of automatic.conf
- Limit message log to one on dnf5 start
- Implement waiting for network for dnf5 automatic
- Write dnf5 commandline to the log
- Implement dnf5-automatic: Tool for managing automatic upgrades
- Parametrize output stream in transaction table
- Add `download --srpm` option
- Add missing dbus signal registations
- Add new versionlock bindings
- Implement `dnf5 versionlock` command

# 5.1.12

- Update translations from weblate
- Drop dnf obsoletion temporarily
- Use regex for tmt plan names
- Add tmt tests identifiers
- PackageQuery: Add `filter_{latest,earliest}_evr_ignore_arch`
- Suggest to use dnf5 command to install dnf5 plugins
- Added arch option to the download command
- CI: Upgrade action/checkout to a version with Node.js 20
- Document explicit nevra remove commands and aliases dropped
- build: Include <unistd.h> for isatty()
- Change user info display on history command to include display name and username
- Revert "Use focusbest: prefer latest deps versions over smaller transactions"
- Fix a warning when building docs.
- modules: Add a test for enabling default modules
- modules: Add a new module stream to test data
- modules: Respect defaults when enabling multiple streams of a module
- modules: Fix TransactionItemType for not found modules
- Build: Require GCC 10.1 for std::in_range<>()
- Add --urlprotocol option to download command
- dnfdaemon: Explicitly specify allowed config overrides
- Disable dnf and dnf5daemon tests
- needs-restarting: get systemd boot time from UnitsLoadStartTimestamp
- doc: Add --destdir option to upgrade command manual
- Move number placeholder to postposition in copr_repo.cpp
- Added url option
- Load protected packages from installroot
- Make protected_packages an append options
- doc: Create a man page for Aliases
- I18N: Annotate literals in advisory command
- Extend filter_release and filter_version tests
- package_query: Fix filter_version with non EQ comparator
- Fix clang format
- Fix code for string deduplication
- Use placeholders to deduplicate strings
- Add __hash__(), __str__(), and __repr__() for Package
- Add __hash__() for Reldep Python binding
- Add __repr__() to python bindings of Reldep
- Define tp_str slot for Reldep Class
- group: Fix using allowerasing option
- Fix misspellings
- I18N: Remove duplicate empty message IDs from catalogs
- I18N: Do not mark empty strings for a translation

# 5.1.11

- Update translations from weblate
- Fix `--skip-unavailable` documentation
- Make `cachedir`, `system_cachedir` relative to `installroot`
- Workaround for swig-4.2.0 missing fragment dependency
- Add `repoquery --recursive` option
- Add `repoquery --providers-of=PACKAGE_ATTRIBUTE` option
- Update documentation of repoquery
- Update documentation for remove command behavior
- Limit search pattern for remove command to NEVRAs and files
- Packaging: Require an exact release of libdnf5-cli by dnf5-plugins
- Disable zchunk on RHEL
- Add dnf5.conf man page
- Add RPM package Group attribute to dnf5daemon-server
- Document changes related to caching
- Document caching man page
- Document Global Option `--help-cmd` dropped
- log_event: Correct message for HINT_ICASE

# 5.1.10

- Document dnf5 plugins
- Document How-to write libdnf5 plugin tutorial
- Document How-to write dnf5 plugin tutorial
- Document Templates for libdnf5 plugin
- Document Templates for dnf5 plugin
- Sort the module info table
- `module info` print hint for active modules
- `module info` print "[a]" for active modules
- Ensure write permission before importing packages
- Change module dependency string to be the same as in dnf4
- `module info`: improve summary and description
- Escape glob characters in pkg specs for `builddep`
- Add `mc` alias for `makecache`
- Implement `logdir`, `log_size` and `log_rotate` config options
- remove redundant "all" in command `check`
- Improve bash completion
- Fix progress bars miss newlines on non-interactive output

# 5.1.9

- Update translations from weblate
- Fix builds for RISC-V arch
- Fix architecture autodetection
- Move `am_i_root` function to common library
- Implement `module info` command
- Add user confirmation request if `history store` overwrites a file
- Add `history store` command
- Add API to serialize base::transaction in JSON
- Add API to serialize transaction::transaction in JSON
- Add docs for `provides`
- Implement command `provides`
- Read `copr.vendor.conf` in `/usr/share` first
- Add docs for `check` command
- Implement `check` command
- Expose `utis/fs/file.hpp` and `temp.hpp` on API
- Document dropping of the `skip-broken` for `upgrade`
- Update man pages with missing dependency resolving-related options
- Document `skip-broken` option only for related commands
- Test for adding an empty list to memory file
- Check serialized temporary files memory is non-empty
- Add `microcode_ctl` to needs-restarting's reboot list
- Fix reporting spec matches only source

# 5.1.8

- Update translations from weblate
- Don't run infinitely when enabling dependent modules and module is not found
- Always print "[d]" in module list for default streams
- Fix transaction table headers for module operations
- Implement `config-manager addrepo --add-or-replace`
- Implement plugin `config-manager`
- Allow globs in module_spec arguments
- Document needs-restarting plugin
- Add no-op `needs-restarting -r` for DNF 4 compat
- Implement `needs-restarting --services`
- Initial implementation of needs-restarting

# 5.1.7

- Actions plugin's actions.conf can set "Enabled" for each action separately
- Actions plugin now supports action options
- Implement `get_reason()` for groups and environments
- Disable the RHSM plugin by default and enable it in the RPM spec
- Add missing docs for `get_advisory_packages_sorted_by_name_arch_evr(bool)`
- Update documentation about maintained coprs
- modules: Test `ModuleProfile::is_default()` method
- modules: Simplify finding whether profile is default in module list
- modules: Fix `ModuleProfile::is_default` method
- modules: Store if profile is default in ModuleProfile object
- Generate docs for undocumented functions so they at least show up
- Add python advisory docs
- Add advisory python API tests
- Enable AdvisoryModule bindings

# 5.1.6

- Document aliases for command line arguments
- Don't print missing positional argument error with `--help`
- Improve error handling for missing arguments
- Document `--forcearch` as a global argument
- Make `--forcearch` a global argument
- Avoid reinstalling installonly packages marked for ERASE
- Add `filter_installonly` to PackageQuery
- Implement new argument `--show-new-leaves`
- advisory: document advisory command changes and few clean ups
- Document `--dump-main-config` and `--dump-repo-config`
- Implement new argument `--dump-repo-config`
- Implement new argument `--dump-main-config`
- Show default profiles in `module list`
- Print hint for the `module list` table
- Show information about default streams in `module list`
- Document `module list` options
- Add `enabled` and `disabled` arguments to `module list`
- Add module spec filtering to `module list`
- Add `module list` command
- Document `group upgrade`

# 5.1.5

- Improved ConfigParser
- Improved docs for `group install` and `group remove`
- Fix man pages deployment
- Update API doc related to keepcache
- Implement `rhsm` (Red Hat Subscription Manager) plugin
- Document `--dump-variables`
- Implement `dnf5 --dump-variables`
- Improve contributing guidelines: don't mention "ready-for-review"
- Allow specifying upper-case tags in `repoquery --queryformat`
- api: Make get_base_arch() public
- Improve input for large epochs that don't fit into `time_t`

# 5.1.4

- Fix Builds on i386
- Print error if unsupported architecture used
- argument_parser: New error class for invalid value
- Allow obsoletion of protected packages
- Add support for repository configuration in /usr

# 5.1.3

- Improved tests for IniParser
- Add directories for repos configuration overrides to the package
- Sort repos in 'repo info' command output
- Fix algorithm in package NEVRA filter
- Add drop-in configuration directories to package
- Make releasever_major, releasever_minor read-only
- Add option conflicts for repoquery
- Improve documentation for `repoquery --{installed,available,updates}`
- changes.rst: `--noautoremove` was added back
- Add `--skip-unavailable` option for `module` commands
- Implement `module disable` and `module reset`
- repoquery: add `--location` and `location` querytag
- repoquery: add `--disable-modular-filtering` option
- Add `dsync` alias for `distro-sync`
- Improved output of `repo_info`

# 5.1.2

- Print error messages in nested errors
- Implement `dnf5daemon-server` introspection xml for Advisory interface
- Implement `dnf5daemon-client advisory info` command
- Implement `dnf5daemon-client advisory list` command
- Implement `dnf5daemon-server` advisory service
- Improve `dnf5daemon-client --help`
- Enable `--repofrompath` repos by default
- Fix error on creating repo with duplicate id

# 5.1.1

- Postpone replace of DNF to Fedora 41
- Add a description of `with_binaries` option for dnf5daemon
- Include RPM logs in KeyImportError
- Abort PGP checking immediately if any checks fail
- Display warning message when any PGP checks skipped
- Don't allow main gpgcheck=0 to override repo config
- gups and environments to `history info` output
- Store missing id and repoid in db for groups/environments
- Fix out-of-bounds access in Goal::Impl::add_install_to_goal
- Fix repoquery `--list`
- `allow_vendor_change` was reverted back to true
- Doc update to allow `logdir` outside the installroot
- Remove `grouplist` and `groupinfo` aliases
- Add `grp` alias for group command
- `repoquery --exactdeps` needs `--whatdepends` or `--whatrequires`
- Update and unify repoquery manpage
- Document replace of `-v` option by `repoinfo` command
- Add `remove --no-autoremove` option
- Document dropped `if` alias of `info` command
- document `actions` plugin
- Fix printing advisories for the running kernel
- Revert "advisory: add running kernel before pkg_specs filtering"

# 5.1.0

- Minor version update. API is considered stabile
- Remove unneeded unused configuration priority
- Don't show dnf5-command hint for unknown options, only commands
- Add hint to install missing command with dnf5-command(<name>)
- Add dnf5-command(<command-name>) provides to dnf5
- Add dnf5-command(<command-name>) provides to dnf5-plugins
- Document several methods as deprecated
- Fix core dump on `--refresh` switch usage
- Add `repoquery -l`/`--list` aliases for `--files` for rpm compat
- Add `vendor` attr to package in `dnfdaemon-server`
- Document `dnf5-plugins` package in man pages

# 5.0.15

- Add `module enable` subcommand
- Add `--repofrompath` option
- Add `--forcearch` option to multiple commands
- Add `reinstall --allowerasing` option
- Add `repoquery --sourcerpm` option
- Add `repoquery --srpm` option
- Add `chacheonly` configuration option
- Add `--cacheonly` option
- Add `--refresh` option
- Change default value for `best` configuration to true
- Change default value for `allow_vendor_change` configuration to false
- changelog: Fix behavior of `--since` option
- builddep: Fix handling BuildRequires in spec files
- swig: Return None for unset options in Python
- Verify transaction PGP signatures automatically
- Fix checking whether updateinfo metadata are required
- Fix handling empty epoch when comparing nevra
- Fix building with upcoming fmt-10 library
- Rename namespace, includes and directories from libdnf to libdnf5

# 5.0.14

- Modify libdnf5-devel to generate pkgconf(libdnf5)
- Handle unnamed environments in transaction table
- Return error exit code on RPM transaction failure
- Add `repoquery --file` option
- Add `repoquery --arch` option
- Add `repoquery --installonly` option
- Add `repoquery --extras`, `--upgrades` and `--recent` options
- Add `repoquery --changelogs` formatting option
- Don't complete ls alias
- Add rq command alias for `repoquery`
- Exclude dnf.conf when not installed
- Improve the download methods API
  - Switch to parameterless download methods and introduce setters for fail_fast and resume
  - Affected classes: libdnf::repo::FileDownloader, libdnf::repo::PackageDownloader

# 5.0.13

- Fix resolve behavior for `download`
- Add a message when `--downloadonly` is used
- Add `--downloadonly` option to multiple commands

# 5.0.12

- Have DNF update to DNF5
  - Add dnf, yum obsoletes and provides
  - Symlinks for `dnf` and `yum` binaries
  - Move ownership of /etc/dnf/dnf.conf, /etc/dnf/vars, and
    /etc/dnf/protected.d from dnf-data to libdnf5
    - Conflict with older versions of dnf-data that own these files/directories
- Print environments in the transaction table
- Add support for environmantal groups in dnf5daemon
- Handle unnamed groups in transaction table
- Update documentation for `distro-sync --skip-unavailable`
- Update documentation for `downgrade --skip-unavailable`
- Update documentation for `upgrade --skip-unavailable`
- Add repoquery `--files` and `files` querytag instead of `--list`
- Add getters to package for: debug, source, repo-name
- Add `repoquery --querytags` option
- Document `repoquery --queryformat`
- Add `repoquery --qf` alias to `repoquery --queryformat`
- Add get_depends() to package and --depends to repoquery
- Implement keepcache functionality (RhBug:2176384)
  - API changes:
    - libdnf::repo::PackageDownloader default ctor dropped (now accepting the Base object)
    - libdnf::base::Transaction not accepting dest_dir anymore (implicitly taken from configuration)
  - A note for existing users:
    - Regardless of the keepcache option, all downloaded packages have been cached up until now.
    - Starting from now, downloaded packages will be kept only until the next successful transaction (keepcache=False by default).
    - To remove all existing packages from the cache, use the `dnf5 clean packages` command.
- goal: Split group specs resolution to separate method
- comps: Possibility to create an empty EnvironmentQuery
- `remove` command accepts `remove spec`
- Refactor remove positional arguments
- Remove duplicates from `group list` output
- Document `copr` plugin command
- Document `builddep` plugin command

# 5.0.11

- Add --contains-pkgs option to group info
- Add filter for containing package names
- Fix parameter names in documentation
- Document create parameter of RelDep::get_id method
- Document RepoQuery::filter_local
- Document repoclosure in man pages
- Document repoclosure command
- Implement repoclosure plugin
- package_query: filter_provides accepts also Reldep
- Fix download callbacks and many segfaults in dnf5daemon
- Add allow-downgrade configuration option

# 5.0.10

- dnf5-plugins: implement 'dnf5 copr'
- Add new configuration option exclude_from_weak_autodetect
- Add new config option exclude_from_weak
- Add repoquery --unneeded
- Fix handling of incorrect argument (RhBug:2192854)
- Add detect_release to public API
- Add group --no-packages option
- Add group upgrade command
- Enable group upgrades in transaction table
- Add --destdir option to download command
- Filter latest per argument for download command
- Add builddep --allowerasing
- download command: filter by priority, latest
- Remove --unneeded option from remove command
- Document autoremove differences from dnf4
- Add autoremove command
- state: Add package_types attribute to GroupState
- comps: Add conversion of PackageType to string(s)
- Add check-update alias for check-upgrade
- Add `check-upgrade --changelogs`

# 5.0.9

- Add `--userinstalled` to `repoquery` man page
- Implement `repoquery -userinstalled`
- Fix: progressbar: Prevent length_error exception (RhBug:2184271)
- Add dnf5-plugins directory in documentation
- Document `repoquery --leaves`
- Implement `repoquery --leaves`
- Implement new filters rpm::filter_leaves and rpm::filter_leaves_groups

# 5.0.8

- Improve error message in download command
- Add repoquery --latest-limit option
- Add dg, in, rei, rm aliases
- Add "up" and "update" aliases for "upgrade" command
- Update documentation with info about package spec expressions (RhBug:2160420)
- Add formatting options repoquery --requires, --provides..
- Remove unused repoquery nevra option
- Add `--queryformat` option to repoquery
- Improved progress bars
- Fix logic of installroot with deduplication
- Correctly load repos from installroot config file
- Improved loading and downloading of key files
- Improved modules: Change State to set and get the whole ModuleState
- New API method rpm::Package::is_available_locally
- Move description of DNF5 changes to doc
- Improved dnf5daemon logic and removed unused code
- Improved progress bar
- Improved handling of obsolete package installation
- Remove showdupesfromrepos config option
- man: Add info about download command destination
- Print resolve logs to stderr
- Fix double loading of system repo in dnf5daemon
- Set a minimal sqlite version
- Change to --use-host-config, warning suggesting --use-host-config
- Add capability to find binaries to resolve_spec
- Add pre-commit file
- Improved by fixing memory leaks
- Improved tests by enabling with multithreading
- Improve documentation  for list command
- Add compatibility alias ls->list
- Implement info command
- Implement list command
- Fix --exactdeps argument description

# 5.0.7

- Document set/get vars in python api
- Document --strict deprecation
- New configuration option "disable_multithreading"
- Improved dnf5daemon to handle support groups and modules in return value
- Ignore inaccessible config unless path specified as --config=...
- Includes reordering and tweaks in advisories
- Add support for package changelogs in swig and tests
- Add many unit tests for dnf5 and python api
- Add new --skip-unavailable command line option
- Add search command
- Add new error for incorrect API usages
- Add a new method whether base was correctly initialized
- Improved python exceptions on undefined var
- transaction: Change API to run transaction without args
- Add explicit package version for libdnf5-cli
- Improved performance of packagequery

# 5.0.6

- Add obsoletes of microdnf
- Many improvements related to internal logic and bugfixes
- Improvements in specfile
- Improved API, drop std::optional
- Use Autoapi instead of Autodoc to generate Python docs
- Improved documentation for modules

# 5.0.5

- Fix build fail in rawhide
- Fixes in the concerning filesystem
- Fixes in the concerning modules
- Fixes in the concerning api

# 5.0.4

- Many fixes in perl bindings
- Test functions enhanced
- Extend unit tests for OptionString and OptionStringList

# 5.0.3

- Add Python docs for: Base, Goal, RepoQuery, Package and PackageQuery
- Add docs for Python bindings: they are auto generated now
- Add --what* and --exactdeps options to repoquery
- Add "user enter password" to dnf5daemon functionalities
- Fix: remove repeating headers in transaction table
- Fix: Set status of download progress bar after successful download
- Fix: RepoDownloader::get_cache_handle: Don't set callbacks in LibrepoHandle
- Refactor internal utils
- Improved GlobalLogger
- Improved C++ API docs

# 5.0.2

- Implement group remove command
- Improved options in config
- Add support for any number of user IDs in a PGP key
- Use new librepo PGP API
- remove gpgme dependency
- Improved exceptions and dnf5 errors
- Add dnf5-devel package
- Update README.md with up to date information
- Repoquery: Add --duplicates option
- Improved documentation for Repoquery, Upgrande and About section
- Add tutorials for python3 bindings
- dnf5-changes-doc: Add more structure using different headings
- Add ModuleQuery
- Improvements in comps logic

# 5.0.1

- Fix loading known keys for RepoGpgme
- Fix dnf5 progress_bar
- Improve modules: conflicting packages, weak resolve, active modules resolving
- plugins.hpp moved away from public headers and improvements logic

# 5.0.0-2~pre

- Fix failing builds for i686 arch

# 5.0.0-1~pre

- Add man pages to dnf5
- Fix non x86_64 builds
- Remove unimplemented commands

# 5.0.0-0~pre

- Dnf pre release build for Fedora
