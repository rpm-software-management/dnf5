# 5.3.0.0

- Update translations from weblate
- Document 5.3.0.0 API changes
- copr: support '$distname' replacement in baseurls
- advisory: Add JSON documentation, standardize timestamps, and suppress error messages in JSON format
- fix: unify epoch display across all DNF5 commands
- doc: mention libpkgmanifest in the CONTRIBUTING.md
- Add JSON output support to history list and info commands
- manifest docs: mark as experimental
- comps: Store comps xml files in groups and environments subdirectories
- comps: Add environment install, remove and upgrade commands
- comps: Add CompsTypePreferred to set preference for comps operations
- comps: Fix descriptions of group and environment specs
- needs-restarting: use std::string to own pid
- needs-restarting: Drop unnecessary logging; display PID and cmdline for -p
- needs-restarting: Correctly use sdbus_compat
- needs-restarting: use org.freedesktop.systemd1.Service interface
- test: Add framework for needs-restarting plugin tests
- needs-restarting: Update the man page to reflect current options
- needs-restarting: Implement the --processes and --exclude-services options
- doc: document output of `--json` for `repo` command
- docs: Use only multiline comments in RST files
- docs: Document vendor change policy TOML config
- Add directories for vendor change policies configuration to the package
- Load vendor change policies from configuration files
- solv::VendorChangeManager: Remove limit on vendor change policies
- Add unit test for "solv::SolvMap::is_intersection_empty"
- Add "solv::SolvMap::is_intersection_empty" for optimized bitwise checks
- New VendorChangeManager: Supports vendor change policies.
- Pool: Set appdata pointer in libsolv pool to its managing solv::Pool
- packit: move getting dnf5 version to package config
- Fix typo in DNF4 and DNF5 documentation
- Drop mentions of dnf5-testing-nightly and dnf5-testing
- offline: Store downloaded packages in /var/lib/dnf
- libdnf5::base::transaction::serialize: don't prefix system repo
- doc: describe repo mangling for `--store` option
- Prefix repository names for `--store`ed transactions
- Simplify a help text for package arguments
- manifest: remove BOOTSTRAP_REPO_ID logic
- manifest: descriptive error for duplicate repository IDs
- manifest: split new and resolve subcommands
- manifest: error when srpm is missing
- manifest: s/source/srpm, s/archs/arch, cleanup
- spec: add `%bcond_without plugin_manifest`
- manifest: port documentation from dnf-plugins-core
- manifest: add --source argument
- manifest: implement existing commands from DNF4 plugin
- manifest: create plugin skeleton
- Fix too long line of new `max_downloads_per_mirror`
- Expose librepo max_downloads_per_mirror configuration
- vars: add doc comments for detect_release, detect_releasevers
- Add --releasever-{major,minor} options
- Override releasever_{major,minor} with provides
- vars: make releasever_{major,minor} writable
- tutorial: libdnf5 plugins: fix `enable` options
- Add `local` plugin
- libdnf5 actions plugin: Document "json" communication mode, polishing
- python_plugins_loader: require and read config files for python plugins
- python_plugins_loader: add `python_plugins_loader.d` config directory
- libdnf5::Base: add back similar API for loading plugins
- Inline `Plugins::load_plugin`, its too small to be worth a separate method
- libdnf5::Base: Separate plugin config loading into a new Base API
- Extend unused `libdnf5::plugin::Plugin` constructor with cfg parser
- python_plugins_loader: Fix invalid path read bug
- python_plugins_loader: update example plugin.py
- python_plugins_loader: pass wrapped `libdnf5::plugin::IPluginData`
- python_plugins_loader: store `libdnf5::plugin::IPluginData`
- python_plugins_loader: the library module is now called `libdnf5`
- python_plugins_loader: fix the name to match the rest of code and rpm pkg
- python_plugins_loader: add default config to enable the plugin
- GHA: Fix weblate-sync-pot workflow
- RepoSack: only attempt fix_group_missing_xml if system_repo loaded
- ConfigMain: deprecate `enabled` option
- daemon: Protect libdnf5 access in D-Bus services with mutex
- libdnf5 actions plugin: Reply with actual values after set configuration
- Add .mailmap to set preferred user names and email addresses
- libdnf5 actions plugin: Add snapshot description in snapper impl example
- doc: Add Tutorial: API changes in callbacks between DNF and DNF5
- test: Add tests for code examples for API changes in callbacks tutorial
- test ConfigMain::load_from_config
- ConfigMain: add load_from_config
- pre-commit: Increase autopep8 line length
- Bump libdnf5-cli rpm and so version
- Add unit tests for long progress bar messages
- cli: Clear the screen instead of padding messages
- progressbar: Do not trim messages to terminal width
- cli: Add pImpl to libdnf5::cli::progressbar::DownloadProgressBar
- cli: Add pImpl to libdnf5::cli::progressbar::ProgressBar
- cli: Add pImpl to libdnf5::cli::progressbar::MultiProgressBar
- feat: add field-specific search options (--name-only, --summary-only)
- Add license header requirement to CONTRIBUTING.md
- Add "Copyright Contributors to the DNF5 project."
- Add SPDX-License-Identifier
- repo: When repo age matches `metadata_expire` exactly expire it

# 5.2.17.0

- Update translations from weblate
- Add `repomanage` plugin
- dnf5daemon-server: Allow RPM key import without password prompt for wheel users
- doc: Add Tutorial: API changes between DNF and DNF5
- test: Add tests for code examples for API changes tutorial
- doc: Format API changes as paragraphs instead of sections
- test: Keep the same working directory when running python tutorial tests
- Introduces `libdnf5` list plugins on `--version` parameter
- cli: Fix use-after-free in NamedArg::parse_short
- cli: Fix use-after-free issue in CommandAlias
- libdnf5: Don't double format a transaction error
- Unify docs for shared transactions options: --offline and --store
- Add `replay` to the hardcoded set of commands that require privileges
- All commands that compose a transaction should support offline and store
- doc: Add a chapter about migrating to dnf5
- doc: changes_from_dnf4: Info about `shell` cmd superseded by `do` cmd
- DNF5: Documentation for "do" command
- dnf5: Move create(_installed)_from_repo_option to public API
- dnf5: builddep: Argument "--from-repo"
- dnf5daemon-server: Correct repo::confirm_key_with_options D-Bus signature
- `--store` option: override keepcache option not to remove packages
- doc: fix references to dnf5.conf(5)
- Replace locale-dependent std::isalnum checks
- repo: Correctly URL-encode packages locations
- test: Add unit tests for common/utils/url
- utils: New url_path_encode() function
- utils: Add url_decode() function
- Move url_encode() to libdnf5::utils::url
- dnfdaemon: Obey downloader D-Bus signal signature

# 5.2.16.0

- Update translations from weblate
- Bump minor version to 5.2.16
- dnfdaemon: Enable download-only transaction execution
- dnfdaemon: Make it possible to set 'interactive' option for Rpm::system_upgrade()
- dnfdaemon: Make it possible to set 'interactive' option for Offline::cancel(), Offline::clean(), Offline::set_finish_action()
- dnfdaemon: Make it possible to set 'interactive' option for Repo::confirm_key(), Repo::enable(), Repo::disable()
- dnfdaemon: Make it possible to set 'interactive' option for Goal::do_transaction()
- dnfdaemon: Make it possible to set 'interactive' option for Base::clean()
- test: Add test for temp files toml with missing key
- repo: Do not fail on empty temporary files toml
- actions: Replace L10N_SSH_KEY secret with RSM CI GitHub token
- dnf5: Support --installed-from-repo= for "do"
- Disable fastest_mirror callback for package downloading
- Copy return value from `SetConstIterator::operator*`
- tests: verify `SetConstIterator` works in Python bindings
- test_nevra.py: change formatting to satisfy autopep8
- Fix various typos found by Lintian
- tests: fix compat with dash and other shellcheck warnings
- dnf5: Support --installed-from-repo= for swap
- dnf5: Support --installed-from-repo= for upgrade/downgrade/distro-sync
- Goal: add_rpm_(upgrade/distro_sync) Improve support from_repo_ids
- Goal: Honor set_from_repo_ids in add_up_down_distrosync_to_goal
- dnf5: Support --installed-from-repo= for reinstall/remove
- Goal: Honor set_from_repo_ids in add_(remove,reinstall)_to_goal
- dnf5: Support --installed-from-repo= for repoquery/list/info
- rpm::PackageQuery: New filter "filter_from_repo_id"
- dnf5: "--from-repo=" option enables and validates source repositories
- Goal::Impl::add_install_to_goal: Fix to_repo_ids handling
- dnf5: upgrade/distro-sync: Apply --from-repo only if pkgs are specified
- Goal::Impl::add_specs_to_goal: Skip processing of empty query in *ALL*
- Goal::Impl::add_specs_to_goal: Apply to_repo_ids settings in *ALL*
- Add unit tests for nevra parsing
- swig: Add bindings for templated functions that convert nevra to string

# 5.2.15.0

- Update translations from weblate
- Adapt missing gpg key check for parallel downloading
- repoclosure: skip rich dependencies
- search: Do not separate a package from a summary with a colon
- Fix: add_up_down_distrosync_to_goal: to_repo_ids filtering
- dnf5: distro-sync: Argument "--from-repo"
- swig: Add %thread directive for repo_sack.hpp for Python
- dnfdaemon: Fix a crash under dnf5daemon::DownloadCB::create_signal_download()
- Add dnf5daemon-server requirement for polkit subpkg
- doc: Correct handling the translations
- dnfdaemon-client: Call server methods asynchronously
- offline: Require D-Bus socket service from dnf5-offline-transaction.service
- offline: Perform gracefull powerof/reboot instead of immediate one
- readthedocs: add swig include to fix doc builds
- readthedocs: extract `SOURCE_DIR` variable
- readthedocs: fix path joining
- dnfdaemon: Make permission check more consistent
- dnfdaemon: Removed incorrect output parameter names
- dnfdaemon: support all_advisories option for recent_changes()
- advisory: Add support for filtering advisories by Nevra
- dnfdaemon: support installonly pkgs in recent_changes()
- dnfdaemon: Add new History interface
- dnf5: repoclosure: Support globs in "--check=" option
- doc: Handling translations
- Docs: enhance install command behavior description
- Unify commands help with package specs
- Introduce new format denoting package spec formats
- test: ease path assertions in tests

# 5.2.14.0

- Update translations from weblate
- Unify system state module saving
- Make system state files writing more robust
- Add `Base` to `system::State`
- automatic: Expand emit_via in command_email emitter to individual arguments
- ci: Remove "Package Build" GitHub Action
- [dnf5] cmdline_aliases: Fix TOML syntax err msg in build TOML11_COMPAT
- doc: aliases: switch code-block language from 'none' to 'TOML'
- cmdline_aliases: Support values consumed by command aliases
- dnf5: Add support for localization of aliases
- dnf5 cmdline_aliases: Print warnings to stderr
- dnf5 cmdline_aliases: Localize printed error messages
- dnf5 cmdline_aliases: Unify use of BasicValue type
- dnf5 cmdline_aliases: Refactor to use shared location functions
- Mark dnf5daemon-server-polkit as noarch
- doc: Correct a typo in dnf-nightly repository URL
- automatic: email emitter: Do not use fractional seconds in e-mail Date headers
- Unify `D-Bus` name formatting
- Expose dnf_daemon dbus Python example in docs
- dnf5: download: Argument "--from-repo"
- dnf5: install/upgrade/reinstall/downgrade/swap: Argument "--from-repo"
- reposync: Spell "GPG" as "OpenPGP" in the usage text
- Update `max_parallel_downloads` doc, it now affects also repo downloads
- Remove no longer needed `RepoDownloader::perform`
- Use new RepoDownloader API to download repo descriptions
- Add new `RepoDownloader` API to donwload only repomd/metalink
- Rename `end_cb` to `end_cb_full_download`, there will be more end cbs
- Move `lr_target`s to `callback_data` and move setup to `add`
- Name `load_repo` function type and add `reusing` option
- Extract `librepo` handle dlist configuration
- Extract error handling of `RepoDownloader::download()` report
- Turn `handle_repo_exception` to `static` `RepoSack::Impl` method
- dnf5: Reset cursor and formatting after SIGINT
- dnf5daemon: Fix a missing reset method if built with sdbus-s++ version 2
- pre-commit: Exclude .tito/tito.props from check-xml
- Add missing interfaces and re-order interfaces in a more logical order
- changes_from_dnf4: makecache doesn't always refresh
- Honor per-repo pgp_gpgcheck=0 even in rpm's enforcing signature mode
- doc: --use-host-config instead of --releasever=/
- dnf5: Command "do": New argument "--from-repo"
- dnf5: Command "do"
- dnfdaemon: Add Polkit rule to allow wheel users to run transactions
- dnfdaemon: Split-off polkit policy for trusted transactions
- doc: Document '\n' sequence in repoquery --queryformat string
- Include dnf-nightly in testing farm testing jobs
- Require new librepo which has fix API for parallel downloads
- Use new API for parallel downloading
- Replace `download_metadata` API with `Add` and `Download`
- Update users of `RepoDownloader` to match new API
- Rework is_repomd_in_sync and is_metalink_in_sync to use new perform
- Make `RepoDownloader::perform` static and rework callbacks
- Convert several of `RepoDownloader` methods that to static ones
- Extract callback data from `RepoDownloader`
- Remove unused `RepoDownloader` defines
- Split `DownloadData` out of `RepoDownloader`
- Include dnf-nightly in packit copr builds
- Fix clang build by disabling ruby deprecation warning
- Fix package build workflow: use code from the tested PR
- doc: Advisory filters can cause transaction commands to fail
- dnf5: Strict handling of --bzs and --cves options
- Add NOT_FOUND_IN_ADVISORIES to resolve log for UPGRADE_ALL
- goal: Report specs filtered out by advisories
- dnf5: Strict handling of --advisories=... option
- dnf5: --no-gpgchecks overrides localpkg_gpgcheck
- I18N: Do not glue sentences in offline command messages
- Remove unused `WITH_ZCHUNK` option
- dnfdaemon: Adjust wording of advisory options descriptions
- dnf5: Adjust wording of advisory options descriptions
- doc: Clarify advisories filters documentation
- expired-pgp-keys: Do not use a deprecated way of removing keys with RPM v6
- doc: "dnf4 config-manager" dropped --dump and --dump-variables options
- Change scriptlet names to match spec designators
- README: Mention #dnf IRC channel in first section
- expired-pgp-keys: Use a temporary GnuPG home directory
- download: Do not print "Downloading Packages:" header in quiet mode

# 5.2.13.1

- RepoCacheRemoveStatistics::get_bytes_removed return uintmax_t
- ci: Use RSM_CI_APP_PRIVATE_KEY in do-release.yaml

# 5.2.13.0

- Update translations from weblate
- daemon: Add the configuration file for the daemon
- repo: Allow system_cachedir copy for the root user
- Fix conditional which assumed RHEL 10 could support SOLVER_FLAG_FOCUS_NEW
- repo: Log correct path when loading system repo
- Unify duplicated group attributes handling
- Add `order_int` support to dnf5 daemon
- Move comps ordering to libdnf5-cli
- Add new group/env API to its adapters and interfaces
- comps: Test merging environment grouplists and optionlists
- comps: Create env grouplist by merging grouplists of all its solvables
- comps: Create group packagelist by merging packagelists of all its solvables
- nevra: Fix description of evrcmp and rpmvercmp functions
- SWIG: unit tests for exceptions
- SWIG: Implement exception forwarding
- libdnf5::FormatDetailLevel: Change to enum class and add doc strings
- Disallow `dnf download non-sense` without `--skip-unavailable`
- Show cleaned space bytes for `dnf clean`
- spec: Remove a duplicate record for %{_prefix}/lib/sysimage/libdnf5
- Fix `history list` with unknown terminal size
- comps: Respect the display_order with listing groups and environments
- comps: Add methods to get the integer value of the display_order
- spec: Correct libdnf5-plugin-appstream description
- dnfdaemon: Allow Base::clean("expire-cache") without admin privileges
- Fix invalid hint for unknown short argument
- Add missing rpm scriptlet types: sysusers and pre/postuntrans
- solv: Drop the size check from swap_considered_map
- Improve missing `proxy_password` message
- Ask for superuser privileges on `history <undo|redo|rollback>`
- RepoDownloader: remove unused code
- expired-pgp-keys: Drop braced initialization to fix clang build
- spec: Fix building without any libdnf5 plugin
- spec: Conditional find_lang calls
- spec: Fix building without dnf5 plugins
- spec: Fix building without man pages
- doc: DNF_SYSTEM_UPGRADE_NO_REBOOT environment variable
- doc: Environment variables for a terminal and temporary files
- Add `-i` and `-f` shoft options for repoquery
- Reimplement `--color` flag
- expired-pgp-keys: Respect install root
- comps: Add configuration options for group and environment excludes
- comps: Add group and environment excludes
- comps: Add comps sack to later store comps excludes
- spec: Package CHANGELOG and other project documentation files
- spec: Set mode to ghost files

# 5.2.12.0

- Update translations from weblate
- repo: handle bad signature errors when no key could be imported
- repo: fix bad GPG error handling
- ci: Automatically backport labeled PRs to stable branches
- packit: remove unnecessary references to main branch
- actions: support releasing from non-main branches
- Implement libdnf5::throw_with_nested: throws our nested exception type
- spec: Move /usr/lib/sysimage/libdnf5 from dnf5 to libdnf5
- libdnf5: OptionBindsError and NevraIncorrectInputError: struct to class
- libdnf5 and libdnf5-cli: Move exception declarations to own header files
- Fix: let copr plugin to respect the installroot option
- Throw error when bootc system is read-only
- Add bootc utility functions
- Bump version to 5.2.12.0
- doc: Templatize dnf-makecache.{timer,service} names
- doc: dnf5-makecache.{timer,service} renamed to dnf-makecache
- Rename dnf5-makecache timer to dnf-makecache when dnf5_obsoletes_dnf
- Fix a memory leak when looking up for an OpenPGP key in RPM database
- Fix dnf5 copr_plugin: always set `base` in `CoprRepo`
- offline: only define systemd constants when building with systemd
- spec: Set cmake minimal version to 3.21
- Remove a warning from a code about an internal use of "dnf5 offline _execute"

# 5.2.11.0

- Update translations from weblate
- offline: Reserve last 5% of progress bar for scriptlets
- offline: Set correct plymouth mode
- offline: Inform user about scriptlets execution
- offline: Correct item number to plymouth message
- offline: Add initial settings of the plymouth
- offline: Fix plymouth ping command
- offline: Remove superfluous [[maybe_unused]] attributes
- Revert "Packit: use GH's release notes for downstream changelog"
- Replace in-tree crc32() with a call to zlib
- doc: fix typo
- Goal: Fix handling duplicit group actions
- progressbar: Messages printing on narrow terminal
- progressbar: Small optimizations
- Goal: Handle INSTALL and INSTALL_BY_COMPS group actions as INSTALL
- Use actual repository ID in stored transactions
- dnf5: Make creating repositories optional
- repo: Invalidate provides after adding a comps xml
- doc: Document dropping of makecache --timer option
- conf: Deprecate metadata_timer_sync option
- system-upgrade: Add --allowerasing switch
- repo: ignore key download errors if skip_if_unavailable
- doc: No value separator after short options
- Don't remove packages/groups during group/environment upgrade
- Goal: handle duplicate group actions
- Remove duplicit clear of `group_specs`
- Update groups and environments during system upgrade
- Group package type can be present but empty
- FindRuby no longer provides upper-case RUBY_* variables.
- doc: Removal of send_error_messages in automatic
- offline: Use connection for creating D-Bus proxies
- offline: Fix constructing vector from D-Bus value
- Enable automatic PR reviews
- expired-pgp-keys: Drop checking for gpg command
- l10n: Rename zh_Hans to zh_CN
- Actions plugin doc: Use inline literals in Output line format section
- Actions plugin documentation: extension in version 1.4.0
- Fix total number of transaction progress bars
- Create progress bar in script callback if one doesn't already exist
- Create a pipe and open files with the close-on-exec flag
- Document Python API by module
- Document cpp API by namespace
- Actions plugin: Increase version to 1.4
- Actions plugin: Support for "log" command in output line in plain mode
- Actions plugin: Action can send stop request or error msg in JSON mode
- Actions plugin: Action can send stop request or error msg in plain mode
- libdnf5 plugin API: Add `StopRequest` class
- Actions plugin: Unify exceptions and log messages
- Actions plugin: Introduce `raise_error` option
- Actions plugin: Move logging code out of child process
- Actions plugin: Create pipes with close-on-exec flag
- expired-pgp-keys: Recommend the plugin only if gpg is already installed
- Fix RepoCache::Impl::remove_recursive: Do not follow symlinks
- Update `expired-pgp-keys` plugin to not use deprecated API
- doc: Add page dedicated to deprecations
- Add runtime warning to `stderr` when calling deprecated API
- Disable deprecation warning for test of deprecated getters
- doc: Add `ConfigMain` to both cpp and python API docs
- Unify marking of depraceted API
- Document `list` command changed handling of installed packages repos
- Fix default value of `pluginpath`
- CMake: Use list(APPEND FOO) over set(FOO ${FOO} ...)
- load_plugins: Preserve original exception with failure information
- Implement *_plugin_get_last_exception in all plugins
- Plugin API: *_plugin_get_last_exception: Return pointer to last exception
- expired-pgp-keys: Install the plugin by default on Fedora 42+
- RepoSack::update_and_load_repos: Properly thread_sack_loader termination
- system::State: remove unnecessary permissions check
- dnf4convert: exit when the database doesn't exist
- Load `libdnf5::system::State` only when required
- Doc: document dropped repoquery and install command variants
- Mark 'Already downloaded' for translations
- plugins: Check if the plugin instantiation was successful
- Library::get_address: Always throw exception if address is NULL
- DNF5 bash completion: "menu completion" support
- DNF5: Support for suggesting command line args without description
- cli::ArgumentParser: Support for suggesting args without description
- daemon-client: Separate context and callbacks
- dnfdaemon: Properly leave event loop
- dnfdaemon-client: Use correct data type for callbacks
- dnfdaemon: Register interface methods for sdbus-cpp-2
- dnfdaemon: Make signal handlers compatible
- dnfdaemon: Explicit sdbus::Variant conversion
- dnfdaemon: sdbus::Variant constructor is explicit
- dnfdaemon: sdbus-cpp v. 2 requires strong types
- cmake: Move sdbus-c++ check to one place

# 5.2.10.0

- Update translations from weblate
- plugins: Provide the actual API version used
- plugins: Check only major version of API for incompatibility
- expired-pgp-keys: New plugin for detecting expired PGP keys
- rpm_signature: Fix rpmdb_lookup comparison case mismatch
- actions: Update with resolved hook
- libdnf plugins: Add resolved hook
- SWIG bindings for common::Message and common::EmptyMessage
- EmptyMessage: class for passing an empty message
- Message: base class for passing a message for formatting in the destination
- utils::format: Support for user defined locale
- SWIG bindings for utils::Locale
- utils::Locale: class for passing C and CPP locale
- utils::format: Support for formatting args according to BgettextMessage
- bgettext: Add function b_gettextmsg_get_plural_id

# 5.2.9.0

- Update translations from weblate
- automatic: Translate end-of-lines in email emitter by DNF
- ruby: Fix swig namespacing in Ruby.
- Correct Ruby %module definition in swig files.
- Documentation enhancements
- Add a hint to `history info` without trans IDs when no match found
- Add `--contains-pkgs=..` option to `history` `list` and `info`
- During package download setup first add all downloads then handle local
- Enhance `perform_control_sequences()` to handle colors
- versionlock: Fix wildcards handling in `add` command
- ruby: Implement Enumerable for libdnf5::advisory::AdvisorySet.
- ruby: Implement Enumerable for libdnf5::rpm::ReldepList.
- ruby: Implement Enumerable for libdnf5::rpm::PackageSet.
- Implement each() for iterating over collection in ruby.
- Add --json output to advisory info
- I18N: Annotate indentation of the transaction summary
- libdnf5: Load plugins with RTLD_NODELETE flag set
- libdnf5: Add a plugin to download and install repo's Appstream data
- Fix bash completion if colon is in the word to complete
- Remove and rename global variables in bash completion
- DNF5 bash completion: Offer package NAMEs in all cases
- Bash completion: always offer NEVRAs for packages
- repo: Fix logging metadata download errors handling
- Copr plugin: Fix resource leak in load_all_configuration
- Own /var/lib/dnf by libdnf5
- Display remaining time as nonnegative number
- automatic: Substitute variables in command_format
- Bumb readthedocs ubuntu image version to fix the docs generation
- automatic: add a default setting to not emit boring messages
- Incorrect library name in libdnf5-cli.pc
- Fix reporting disk space to be freed on a pure package removal
- Support ProgressBar messages with wide characters
- Add padding to ProgressBar messages to avoid overlapping
- SWIG: support repo::DownloadCallbacks user_data
- Remove redundant %python_provide statements
- python3-libdnf5: Remove superfluous provides for python-libdnf
- Update pre-commit hooks to latest versions in F41

# 5.2.8.1

- Update translations from weblate
- doc: Replace another instance of "PGP" with "OpenPGP"
- doc: Use OpenPGP instead of PGP
- Python API: Method DownloadCallbacks.add_new_download can return None
- changes_from_dnf4: fix formatting of indented `list` points
- test: add progressbar tests for interactive mode
- test: enhance progressbar tests for non interactive mode
- test: add `ASSERT_MATCHES` for convenient fnmatch pattern matching
- When determining interactivity add `DNF5_FORCE_INTERACTIVE` override
- Fix overwriting of old output from `MultiProgressBar`
- Add `cursor_down` TTY_COMMAND
- Fix new line printing for unfinished Total progress bar
- Remove new line printing fix in non-interactive mode
- reposync: Do not allow --safe-write-path with multiple repos.
- reposync: Optimization of internal structures
- reposync: Rename --source to --srpm
- Implement reposync plugin
- builddep: Fix changes_from_dnf4 documentation
- builddep: Use enum to determine argument type
- builddep: Add support for --spec and --srpm options
- Fix libdnf5 actions plugin sign conversion compilation err

# 5.2.8.0

- Update translations from weblate
- doc: Use PGP instead of GPG
- Install defs.h for /usr/include/dnf5/context.hpp
- Download cmd: Require at leats one argument/package to download
- Fix copr chroot specification: replace faulty regex with simpler split
- Add packit job to run ABI check plan on testing farm
- changelog_plugin: Limit required metadata to "other"
- doc: Document new `all` optional_metadata_types value
- Accept also "all" whenever optional metadata are checked
- repo: Add option to download all repository metadata
- rpm: New API to check PGP signature of RPM file
- spec: toggle dnf5_obsoletes_dnf for RHEL 11
- repo: While cloning root metadata copy also metalink
- repo: Make Repo::download_metadata() method public
- I18N: Mark <unknown> message in dnf list --installed output for a translation
- package_downloader: Ensure creation of intermediate directories
- Make `test_multi_progress_bar` test more resilient
- I18N: Mark "Total" message in MultiProgressBar() for a translation
- builddep: add support for remote arguments
- Hint when an unknown option is available on different commands
- reformatting to meet clang-format requirements
- add missing include
- I18N: Mark messages in "dnf search" output for a translation
- Extend unit tests with `user_cb_data` in `repo::DownloadCallbacks`
- SWIG bindings for `user_cb_data` in `repo::DownloadCallbacks`
- Basic Perl unit tests for DownloadCallbacks and RepoCallbacks
- Perl unit tests: Clean package_query tests, use BaseTestCase
- Make `BaseTestCase` for Perl unit test
- rpm: Reset RPM log callback upon RpmLogGuard destruction

# 5.2.7.0

- Update translations from weblate
- dnfdaemon: resolve and do_transaction cannot run simultaneously
- dnfdaemon: Base.reset() API
- dnfdaemon: Call base.setup() after setting releasever
- dnfdaemon: Move base initialization to special method
- dnfdaemon: Store session goal in unique_ptr
- dnfdaemon: Add transaction mutex to the Session class
- daemon tests: Adjust tests to current behavior
- daemon tests: Drop test_repoconf.py
- repo: Fix invalid free()
- Optimise multi_progress_bar tty control sequences
- MultiProgressBar now buffers the output text to a single write
- Clear up changes doc about optional subcommands
- PackageDownloader unit tests: Number of add_new_download and end calls
- DownloadCallbacks: Ensure `end` for every successful `add_new_download`
- Behave more like the old service, with the "--timer" option.
- Run "makecache" periodically to keep the cache ready.
- Add couple progress bar unit tests
- When `multi_progress_bar` finishes print new line automatically
- Fix parsing of offline transaction JSON file
- Optimize getting counts of transaction items
- Add `get_base()` to `libdnf::transaction::Transaction`
- historydb: Prevent insertion of duplicate group packages
- Update dnf5.conf.5 to reflect change in fastestmirror behavior
- Return `ConfigMain::get_errorlevel_option()` API
- build: Remove an explicit swig option -ruby
- Document dropped `--disableexcludepkgs` option
- Drop `errorlevel` config option
- Test that both pkg_gpgcheck and gpgcheck point to the same OptionBool
- Change `gpgcheck` option to `pkg_gpgcheck` but stay compatible
- chore: Clean up Fedora 37-related conditionals in RPM spec
- log: Preserve log messages during RPM transaction
- Recommend --use-host-config if --installroot is used and not all repositories can be enabled
- test: Normalize Python code
- Make most descriptions for dnf5 --help translatable.
- test python3: improve comps tests
- comps: add get_base() to {Group,Environment}{,Query}
- doc: "dnf repoquery --unsatisfied" is not supported
- Unit tests for libdnf5::utils::[is_glob_pattern | is_file_pattern]
- [swig] Bindings for libdnf5::utils::[is_glob_pattern | is_file_pattern]
- Allow unlimited number of arguments for history `list` and `info`
- `history store` command: move arg verificion above user confirmation
- libdnf5::utils::patterns: No inline API functions, mark noexcept
- Fix libdnf5::utils::patterns: Include missing headers
- When writing main solv file (primary.xml) don't store filelists
- Set `POOL_FLAG_ADDFILEPROVIDESFILTERED` only when not loading filelists
- i18n: Fix plural forms for "Warning: skipped PGP checks..." message
- i18n: Update translation templates from Weblate
- dnf5: OfflineExecuteCommand needs to set transaction
- dnf5: Print RPM logs to the user
- dnf5: Refactor scriptlet output printing
- dnf5: script_output_to_progress() return number of lines
- rpm.Transaction: Handle RPM transaction log messages
- transaction: Store RPM log messages
- RpmLogGuard: Add a buffer for recently emitted logs
- comps: Fix memory issues in group serialization
- Add --allmirros option for `dnf download --url`
- Consistently use "removing" instead of "erasing" packages
- dnfdaemon: D-Bus API to reset the goal
- dnfdaemon: Add reset_goal() method
- goal: Add missing members to reset() method
- man: Package changes from DNF4 doc as man page
- doc: Rename the file documenting changes from DNF4
- copr: use pubkey URL returned by Copr API

# 5.2.6.2

- Update translations from weblate
- [libdnf, actions plugin] Pipe and OnScopeExit in execute_command
- [libdnf, actions plugin] Check if the received JSON request is allowed
- [libdnf, actions plugin] Add location of hook definition to logged mesg
- [libdnf, actions plugin] Implement "json" communication mode
- [libdnf, actions plugin] Support for "mode" option in action line
- [libdnf, actions plugin] substitute: Fix detection of repos config option
- setlocale: If locale setting fails, try using C.UTF-8 as fallback
- Do not install /var/cache/libdnf5 directory
- dnf5daemon-server: Run transaction test for offline transactions
- Fix deserialization when checking signals `object_path`
- chore: static_cast to fix sign conversion warning

# 5.2.6.1

- Update translations from weblate
- dnf clean: Do not report an error on a nonexistent cache directory
- build: Add -Wsign-conversion to CXXFLAGS
- Fix a sign propagation in calculating transaction size statistics
- dnf5: Check offline transaction state before download
- dnf5: Run transaction test for offline transactions
- I18N: Mark messages in "dnf info" output for a translation
- Fix: libdnf5-cli: TransactionSummary counters data type
- doc: Document arch override for API users
- I18N: Mark messages in "dnf install" output for a translation
- swig: Add wrappers for TransactionEnvironment and TransactionGroup
- doc: add typical dnf5 workflow
- doc: generate `ArgumentParser` c++ docs
- doc: generate `Vars` c++ and python docs
- doc: add missing python ConfigRepo to match c++
- doc: add RepoSack to both cpp and python docs
- doc: fix order in api/c++/libdnf5.rst
- offline: Update usage of toml11-devel
- transaction_callbacks: Deprecate libnf5::rpm::TransactionItem alias
- spec: Recommend dnf5-plugins if dnf-plugins-core installed
- automatic: Use original dnf4 config file location
- doc: dnf5-repoquery: Mention %{reason} query tag at --userinstalled

# 5.2.6.0

- Update translations from weblate
- dnfdaemon: Correct argument direction in interface definition of Goal::cancel method
- dnfdaemon: Signals should use "object_path" type for the session_object_path argument
- rpm: TransactionCallbacks class documentation
- doc: Add TransactionCallback class to API docs
- main: Don't use Context::print_error when catching libdnf5 errors
- Fix transaction problem formatting
- config-manager: Fix addrepo from-repofile with empty/comment lines
- dnfdaemon: API to cancel current running transaction
- DownloadCallbacks: Enum for possible return codes
- Use Context::print_output, Context::print_error in main.cpp
- context: Add print_error and print_output
- Context::set_output_stream should set out_stream & err_stream
- Move some output from stdout to stderr
- Print "info" and progress bars to stderr, not stdout
- conf: clarify plugin types
- conf: update documented options
- Document dropping of `arch` and `basearch` conf options
- conf-todo: remove already documented options
- conf: document both repo and main `gpgcheck` option
- conf: document several `repo` options
- conf: `reposdir` add missing default
- dnf5: Print rpm scriptlets outputs to the user
- transaction: Store outputs of the last rpm scriptlet
- rpm::transaction: Base::Transaction to callback holder
- Install `defs.h` include for `libdnf5-cli`
- [dnf5] Add argument "-c" - alias to "--config" (dnf4 compatibility)
- fix formatting issues
- fix: address various issues mentioned in PR review
- feat: implement a proper default user-agent string
- doc: Add references from dnf5(8) to new man pages
- doc: Add man page describing the system state
- state: Catch all errors during system state load
- state.cpp: update to new toml11-devel-4.0.0 version API
- versionlock_config.cpp: update to new toml11-devel-4.0.0 version API
- temp_files_memory.cpp: fully specify arg for toml::format
- cmdline_aliases: update to `toml11-devel-4.0.0` API
- spec: fix modularity man page
- doc: Include modularity documentation
- doc: Revise packages filtering doc section
- package_downloader: Always call download callbacks
- repo: New Repo.get_packages_download_dir() method
- doc: Document destdir main config option
- doc: Naming of source and debug repos
- doc: fix arguments for `install`, `upgrade` and `remove`
- doc: add `environment-spec` to specs.7.rst
- doc: link `group_package_types` with install and group install
- doc: Use ~ instead of /home/$USER
- Update the man page entry for the countme option
- Fix countme bucket calculation
- Fix up some comments in addCountmeFlag()
- doc: enhance `logdir` default description
- doc: describe `cachedir` and `system_cachedir` relationship
- doc: Add system-upgrade example using D-Bus API
- dnf5daemon-client: system-upgrade command
- dnfdaemon: Add system_upgrade() method to Rpm interface
- spec: fix cmake focus_new arg
- Use `SOLVER_FLAG_FOCUS_NEW` to install latests versions of deps
- builddep: Support the --with/--without options to toggle bconds
- dnf5: Reduce the noise around running scriptlets
- cli: Method to drop the last progress bar message
- spec: Stricten a dependency on DNF libraries in plugin subpackages
- Fix a use-after-free in EmitterEmail::notify()
- Clarify `group` command man page: `environment` note
- Add some docs about environments
- dnfdaemon-client: Fix repoquery command
- dnfdaemon-client: Clean command implemetation
- dnfdaemon: D-Bus API for cleaning caches
- Add an example how to disable repo to `repo` command man page
- Improve "After this operation" disk usage messages
- replay: Allow up/down-grade if NA is already installed
- Add `redo` to and install history documentation
- Add `override_reasons` to `GoalJobSettings` and use it in `redo`
- Add `history redo` command
- Extract private API `to_replay()` for `libdnf5::transaction::Transaction`
- Make `get_package_types()` from libdnf5::transaction::CompsGroup public
- package_downloader: Add local files to callbacks
- dnf5: Offline transactions work with local rpm files
- package_downloader: Handle local files
- transaction: Flag whether download local packages

# 5.2.5.0

- Update translations from weblate
- Use the same nevra format for history and regular transactions
- `history store` add user friendly message when operation succeeds
- Improve `history store` message to mention transaction ID
- Rename wrongly names `query_format` name to `output_arg`
- Store to the db only requested package types for group
- Update `group_package_types` docs to match dnf5
- When upgrading a group keep stored package types
- transaction_sr: Remove excessive "." from exceptions
- TransactionReplay: handle group package types
- dnfdaemon: Replace check_pending() with get_status()
- dnfdaemon: Strict set_finish_action() value check
- dnfdaemon: Add new API for offline transactions
- dnfdaemon-client: --offline option for transactions
- dnfdaemon: Support for running a transaction offline
- Documentation: ABI: Defining public (exported) symbols
- Fix DNF5: Don't trigger filelists download if abs path to local RPM
- Add `history rollback` command
- Use transaction merging when reverting transactions
- Add private `merge_transactions(...)` API
- Add `MERGE` goal action and `MERGE_ERROR` goal problem
- Add reports when corresponding debug package is not available
- Implement conditional compilation `-DWITH_MODULEMD=OFF`
- Add a copr build with disabled modules to verify it works
- Remove unused includes in test_modules
- Clean up `repo` header includes
- Add documentation of debuginfo-install command
- Add debuginfo-install command
- Add install_debug to goal
- Add method to enable debug repository
- Support colon in username, use LRO_USERNAME and LRO_PASSWORD

# 5.2.4.0

- Update translations from weblate
- Add "Complete!" message after succesfull transaction
- Handle exceptions when parsing replay `JSON`
- `TransactionReplayError` move to header `transaction_sr.hpp`
- Add `replay` command to replay stored transactions
- Add const `TRANSACTION_JSON` definition in one place
- Update `history store` `output` option to use directories
- [libdnf, actions plugin] Documentation: get/set repositories options
- [libdnf, actions plugin] Support get/set repositories options, ver 1.1.0
- [libdnf, actions plugin] Use enum for pipe ends instead of magic values
- docs: Update nightly copr repo name
- Add JSON output to advisory list
- `undo`: document new options change in comparison to dnf4
- Make goal action strings translatable
- Fix a typo in log_event message
- `undo`: update history man page
- builddep: Add build-dep alias
- Fix: dnf5 builddep plugin: Link with "common"
- daemon: Generate transfer_id on server side
- Re-enable unit tests that use hidden (private) libdnf5 symbols
- libdnf5: Build static libdnf5 library, use it in unit tests
- Fix: Export symbol goal_action_is_replay
- Do not export symbols from private "/common/utils"
- dnf5 app, dnf5 plugins: Do not export private symbols
- dnf5: Define macros for assigning symbol visibility
- libdnf5-cli library: Do not export private symbols
- libdnf5-cli: Define macros for assigning symbol visibility
- Disable libdnf5, libdnf5-cli, dnf5 cpr_plugin C++ unit tests
- libdnf5 library, libdnf5 plugins: Do not export private symbols
- libdnf5: Define macros for assigning symbol visibility
- libdnf5: API: Do not use inline methods to call private methods
- replay: make condition check strict and use parsed nevra
- Add clarifying comments about path to replay
- Turn reverting of comps upgrade into a warning
- Add hints for `--ignore-installed` and `--ignore-extras`
- Transaction replay: add checking for extra packages
- Enahnce INSTALLED_IN_DIFFERENT_VERSION for reinstall with available pkgs
- Add replay goal actions to `LogEvent` messages
- Use `ignore_installed` option in transaction replay
- Add clarifying comment for Replace action in transaction replay
- Add clarifying comments to PackageReplay
- Add autocomplete for history info
- Add `ignore_installed` and `ignore_extras` to `GoalElements`
- Add `history undo` command
- Allow specifying number of repeats for `TransactionSpecArguments`
- Goal: Add API for reverting history transactions
- Split out `add_replay_to_goal` from adding serialized trans
- Add `transaction_item_reason_at(...)` to get history reason
- offline: Add pImpl for OfflineState classes
- transaction: Docstrings for offline state classes
- dnf5: Remove unused variable
- offline: Add offline update magic symlink constant
- Move offline from dnf5 to libdnf5
- main: Move download callbacks setup after cmd parsing
- context: Set quiet mode on json output
- repoinfo: Implement json output
- repolist: Implement json output
- context: Prepare shared json option
- Update description of --minimal option
- Add --minimal option for check-ugrade command
- spec: Fix files and directories ownership

# 5.2.3.0

- Update translations from weblate
- const: Shared constant defining RPM transaction lock file
- main: Implement checking of privileges before executing commands
- exception: Add new exception for user insufficient privileges
- locker: Move to public API
- distro-sync: Add downloadonly option
- commands: Fix using store option
- Enhance warning about RPMs that were not validate by RPM

# 5.2.2.0

- Update translations from weblate
- dnf5daemon: The buildtime attribute has been added to the package_attrs option
- docs: Document changes to repoinfo and repolist
- packit: Create downstream PR only for Rawhide
- prepare-release: Use new v1.2 action with 4-digits version format
- test: Unit tests for append option
- conf: Remove unused option_T_list_append template
- conf: Convert append options to use new classes
- bindings: Add new append option classes
- conf: New classes for append options
- base: Enhance installed pkgs solver problems reporting
- base: Remove trailing space from RULE_UPDATE message
- base: Deduplicate solver problem messages
- base: Add repository to solver problem messages
- i18n: Unwind "Cannot {} package \"{}\"" message
- fix: quote `dnf5-command({})' in command suggestion when plugin not found
- automatic: Clarify changes dnf4 vs dnf5
- automatic: ship default automatic.conf
- automatic: Fix the documentation
- automatic: Adjust config files reading behavior
- automatic: Fix random_sleep option
- download: add `--source` alias for `--srpm`
- needs_restarting: Fix invalid reference usage
- doc: Improve docs regarding the keepcache option and download command
- Option `--providers-of` doesn't require available repos
- [DOC, libdnf, actions plugin]: Add new hooks to documentation
- [libdnf, actions plugin] Support `pre/post_add_cmdline_packages` hooks
- [libdnf, actions plugin] Support `repos_loaded` hook
- [libdnf, actions plugin] Support `repos_configured` hook
- [libdnf, actions plugin] Mark Action class as final
- Fix a typo in the message
- `history list`: count also groups and envs in total `Altered`
- i18n: Unwind "No {} to remove for argument: {}"
- Add/fix documentation for rpm::PackageQuery methods
- i18n: Improve formatting an error message for multiple streams
- dnf5daemon: Add skipped packages to transaction table
- cli: Split transaction table into sections
- cli: Add skipped packages to the transaction table
- transaction: Add methods to get skipped packages
- transaction: Move process_solver_problems into Impl
- docs: correct the default for pluginconfpath
- Vars::substitute: fix use-out-of-scope leaks

# 5.2.1.0

- Update translations from weblate
- bindings: Tests for using struct attributes in Python
- bindings: Add Python attributes for structs
- docs: Fix diff link on the dnf 5.2.0.0 changes page
- docs: Add diff with API changes in dnf5-5.2.0.0
- docs: Add a page about public API changes in dnf 5.2.0.0
- system-upgrade: fix missing \n before transaction test
- system-upgrade: comment to clarify progress bar logic
- system-upgrade: drop [[maybe_unused]] from reboot() arg
- system-upgrade: fix progress bars, set transaction description
- system-upgrade: adapt to new transaction serialization format
- system-upgrade: clean up releasever logic
- system-upgrade: fix poweroff_after
- copr: the dnf5 copr enable sets CoprRepoPart.enabled = true
- Add file search result for repoquery --whatprovides
- doc: Add enviroment variables and clarify options for loading the plugins
- dnfdaemon: Fix Rpm interface introspection file

# 5.2.0.0

- Update translations from weblate
- [DNF5] `--enable-plugin` and `--disable-pluin`: no match found message
- [DNF5] API: Move Context::libdnf5_plugins_enablement to p_impl
- spec: Add conflict with the former provider of plugin man pages
- spec: Add conflict with the old provider of dnf.conf
- [DNF5] Fix: Remove transaction_store_path from public, add getter/setter
- [libdnf5 API] Base::get_plugins_info
- [libdnf5 plugins] include iplugin.hpp in plugins instead of base.hpp
- repo_sack: Treat all repos with solv_repo created as loaded (RhBug:2275530)
- [DNF5] API: No inline methods in shared_options.hpp
- [DNF5] API: offline::OfflineTransactionState: no inline methods, move cpp
- [DNF5] API: Remove unused and buggy RpmTransactionItem class
- [DNF5] Command: no inline methods
- [DNF5] API: Context: add p_impl, move public vars to p_impl, getters
- API: cli::session: no inline methods and public vars in opts classes
- API: cli::session::Command: no inline methods
- API: add p_impl to cli::session::Session
- API: rpm::TransactionCallbacks: no inline methods
- API: repo::RepoCallbacks: no inline methods
- Prepare for switch of dnf5 in Rawhide
- base: Make get_transaction_history unstable
- Set `group` reason for packages removed by a group removal
- [DNF5] Implement `--enable-plugin` and `--disable-pluin`
- [libdnf5 API] Base::enable_disable_plugins
- spec: Simplify man page files
- Loggers: Fix: Add missing "null_loger.cpp" file
- Loggers API: unify, explicit ctors, non-inline methods, use p_impl
- doc: Review of DNF4 vs DNF5 CLI and configuration changes
- Re-enable clang builds after API changes
- Add `--store` option for storing arbitrary transaction
- libdnf5::Goal: when adding serialized transaction accept local items
- Goal: change `add_serialized_transaction()` to accept path to trans
- Add group/env paths for transaction parsing/serializing
- base::Transaction: during serialization allow specifying paths
- base::Transaction: add `store_comps(...)` method
- repo_sack: add stored_transaction repo and its private API
- repo: add private API `add_xml_comps(path)`
- Generalize logging of `read_group_solvable_from_xml(..)`
- Add `environment_no_groups` to `GoalJobSettings`
- libdnf5 IPlugin: Pass IPluginData instead of Base to constructor
- libdnf5 IPlugin: Use pImpl
- libdnf5 IPlugin: Do not use inline methods
- dnf5 IPlugin: Do not use inline methods on API
- libdnf5 IPlugin: Add argumets description
- libdnf5 plugins: New hooks `pre/post_add_cmdline_packages`
- libdnf5 plugins: New hook `repos_loaded`
- libdnf5 plugins: New hook `repos_configured`
- Base: notify_repos_configured and are_repos_configured methods
- Fix: implicit conversion changes signedness, unused value
- Disable unit tests for Copr dnf5 plugin
- dnfdaemon: Document Polit CheckAuthorization call
- dnfdaemon: Catch timeout during CheckAuthorization
- Not handle compatibility.conf as configuration file
- config: add search (se) and info (if) aliases
- Improve documentation of repo config directories
- Cross reference documentation
- Document Repos and Vars Dirs
- doc: Unify style and move "Files" section
- Document repos configuration overrides
- doc: Remove ":" in titles
- Bump libdnf5/libdnf5-cli so version
- Mark multiple strings for translation
- Set locale for dnf5 run
- spec: Add missing dnf-config-manager.8.gz file
- Generate documentation for ConfigRepo Class
- [Doc] Describe denerating repo cache path
- dnf5daemon: Make availability case insensitive
- dnf5: Drop unneeded severities capitalization
- dnf5: Document --available as default for advisory cmd
- dnf5daemon-client: Drop unneeded severities capitalization
- advisory: filter_severity and filter_type case insensitive
- dnfdaemon: Fix and enhance Advisory interface doc
- Enable import data from DNF4 for systems without state dir
- libdnf5 options: Unify constructors - pass args for storing by value
- dnfdaemon: Missing signal registration
- doc: config manager plugin: wrap too long lines
- doc: document config-manager plugin
- Packit: get version from specfile for copr_builds against main
- Update tests to use new `load_repos()` API
- Use new load_repos instead of deprecated update_and_load_enabled_repos
- Make `libdnf5::repo::Repo::load()` private
- Deprecate: `update_and_load_enabled_repos`
- RepoSack: add new `load_repos` method
- Move update_and_load_repos and fix_group_missing_xml to Impl
- Prevent loading plugins for unittests
- Respect plugins configuration option for loading plugins
- Add pImpl to `libdnf5::LogRouter`
- Add pImpl to `libdnf5::MemoryBufferLogger`
- Add pImpl to `libdnf5::OptionBinds`
- Add pImpl to `libdnf5::OptionBinds::Item`
- Add pImpl to `libdnf5::Config`
- Add pImpl to `libdnf5::OptionStringList`
- OptionStringList: remove assignment operators and move constructor
- Add pImpl to `libdnf5::OptionBool`
- OptionBool: remove assignment operators and move constructor
- Add pImpl to `libdnf5::OptionNumber`
- Add pImpl to `libdnf5::OptionPath`
- Add pImpl to `libdnf5::OptionString`
- libdnf5::OptionEnum: remove template, add pImpl
- Add pImpl to `libdnf5::Option`
- modules: Report problems with switching module streams
- modules: Report switched module streams
- modules: Add switching module streams as a possible transaction action
- modules: Add replaces and replaced_by to TransactionModule
- Add missing info updates alias, to match list command
- Update `package_info_sections` not to use `scols_table_print_range`
- libdnf-cli: Extract package info printing
- ArgumentParser: use p_impl, no inline methods
- ArgumentParser:PositionalArg: Unit tests: Support repeating of pos arg
- ArgumentParser:PositionalArg: Support repeating of positional argument
- Hide/Remove deprecated `libdnf5::repo::Repo` API
- Remove deprecated members from `/include/libdnf5/logger/factory.hpp`
- Remove deprecated unused function `create_forcearch_option()`
- builddep: Don't escape globs, use expand_globs = false
- builddep: Don't try to expand globs in pkg specs
- libdnf5-cli::output: Use ifaces instead templates. Move code to .cpp files
- Interfaces and adapters
- module::ModuleStatus: Move to separate header file
- comps::PackageType: Move to separate header file
- cmp_naevr: Fix: pass by reference
- modules: Report module solver problems
- Accept SolverProblems for transacion resolve log
- modules: Return problems from the module solver
- modules: Add a method to process module solver problems
- modules: Add a separate set of problem rules for modules
- modules: Store the original module context also in the libsolv solvable
- modules: Internalize modular repositories
- Add pImpl to `libdnf5::repo::RepoCache` and `RepoCacheRemoveStatistics`
- Add pImpl to `libdnf5::repo::RepoQuery`
- Hide deprecated `libdnf5::base::with_config_file_path` into Impl
- Remove deprecated `libdnf5::Base::load_config_from_file`
- Move all `libdnf5::Base` members to pImpl
- Add pImpl to `libdnf5::rpm::Reldep`
- Add pImpl to `libdnf5::rpm::Changelog`
- Add pImpl to `libdnf5::rpm::Nevra`
- Add pImpl to `libdnf5::rpm::Checksum`
- Add pImpl to `libdnf5::rpm::Package`
- Adjust code to new rpm::PackageQuery::filter_* methods after the rebase
- Add a method accepting std::string for filter_repo_id()
- Add a method accepting std::string for filter_location()
- Add a method accepting std::string for filter_file()
- Add a method accepting std::string for filter_supplements()
- Add a method accepting std::string for filter_enhances()
- Add a method accepting std::string for filter_suggests()
- Add a method accepting std::string for filter_recommends()
- Add a method accepting std::string for filter_obsoletes()
- Add a method accepting std::string for filter_conflicts()
- Add a method accepting std::string for filter_requires()
- Add a method accepting std::string for filter_description()
- Add a method accepting std::string for filter_summary()
- Add a method accepting std::string for filter_url()
- Add a method accepting std::string for filter_sourcerpm()
- Add a method accepting std::string for filter_nevra()
- Add a method accepting std::string for filter_evr()
- Add a method accepting std::string for filter_arch()
- Add a method accepting std::string for filter_release()
- Add a method accepting std::string for filter_version()
- Add a method accepting std::string and int for filter_epoch()
- Add a method accepting std::string for filter_name()
- Add a method accepting std::string for filter_provides()
- Extend version to four numbers (5.x.y.z)
- Unify smallest version number name
- cmake: rename PROJECT_VERSION_* to just VERSION_*
- dnf5daemon: Document before_begin / after_complete signals
- dnf5daemon: Signals to wrap rpm transaction execution
- rpm: New callback to wrap whole rpm transaction
- Add pImpl to `libdnf5::module::ModuleProfile`
- Add pImpl to `libdnf5::module::ModuleDependency`
- Add pImpl to `libdnf5::module::Nsvcap`
- Add pImpl to `libdnf5::module::ModuleQuery`
- Add pImpl to `libdnf5::comps::EnvironmentQuery`
- Add pImpl to `libdnf5::comps::GroupQuery`
- Add pImpl to `libdnf5::comps::Environment`
- Add pImpl to `libdnf5::comps::Package`
- Add pImpl to `libdnf5::comps::Group`
- Remove unused `libdnf5::comps::GroupSack`
- Remove unused `libdnf5::comps::EnvironmentSack`
- Remove unused `libdnf5::comps::Comps`
- Add pImpl to `libdnf5::advisory::Advisory`
- Add pImpl to `libdnf5::advisory::AdvisoryReference`
- Add pImpl to `libdnf5::advisory::AdvisoryCollection`
- Add pImpl to `libdnf5::advisory::AdvisoryQuery`
- Add pImpl to `libdnf5::rpm::RpmSignature`
- Add pImpl to `libdnf5::transaction::Transaction`
- Add pImpl to `libdnf5::rpm::KeyInfo`
- Add pImpl to `libdnf5::repo::RepoSack`
- Add pImpl to `libdnf5::base::SolverProblems`
- Add pImpl to `libdnf5::base::LogEvent`
- Add pImpl to `libdnf5::ConfigParser`
- Add pImpl to `libdnf5::Vars`
- Add pImpl to `libdnf5::transaction::TransactionHistory`
- Add pImpl to `libdnf5::transaction::Package`
- Add pImpl to `libdnf5::transaction::CompsGroup`
- Add pImpl to `libdnf5::transaction::CompsEnvironment`
- Add pImpl to `libdnf5::transaction::TransactionItem`
- Remove several not needed imports
- repo: add p_Impl and several needed utility methods
- Repo: remove unused `fresh()` and `timestamp` attribute
- Add pImpl to `ModuleItem` and remove definitions from header
- Add pImpl to `libdnf5::base::transaction_*` classes
- Add p_impl to libdnf5::GoalJobSettings and add getters and setters
- Add p_impl to libdnf5::ResolveSpecSettings and add getters and setters
- dnf5: bash completion: Prefer using "_comp_initialize" with fallback
- dnf5: Bash completion: Switch to `_init_completion`
- Fix `DISTRO_SYNC_ALL` (distro-sync without arguments, system upgrade)
- Support RPMTRANS_FLAG_DEPLOOPS
- Give inline methods hidden visibility by default
- dnfdaemon: Make only internally used funcs static
- dnfdaemon: Enhance Rpm.list() / Rpm.list_fd() documentation
- doc: Add example of Rpm.list_fd usage in Python
- dnf5daemon-client: Repoquery uses new Rpm:list_fd() API
- dnfdaemon: New method list_fd() on Rpm interface
- dnfdaemon: Serialize package object to JSON string
- dnf5daemon: Handler that return data using UNIX_FD
- dnf5daemon: Auxiliary method to write string to fd
- dnf5daemon: Move utils functions into dnfdaemon namespace
- dnf5daemon: get_session() method for D-Bus services
- dnf5daemon-server: Ignore SIGPIPE
- Vars: Add unit tests for API methods
- Vars::unset: API method for removing variable
- dnf5daemon-server/dbus: Install config files into /usr
- Fix: libdnf5-cli::output::action_color: Move implementation to .cpp file
- Fix: Do not use Variable-length arrays (VLAs) in C++ code
- Add a hint to call base.setup() prior loading repositories
- dnf5daemon-client: New switches for group list
- doc: Include comps.Group interface to D-Bus API documentatin
- dnfdaemon: Enhance comps.Group.list() method
- dnf5daemon-client: Fix group.get_installed()
- man: Link dnf5 pages to dnf

# 5.1.17

- dnf5daemon: Remove reposdir from allowed config overrides
- Update translations from weblate

# 5.1.16

- Update translations from weblate
- Document system-upgrade aliases
- Improved Bash Completion
- Print command line hints after resolve failure
- Docuent Advisory.list() API usage
- Add NEVRA field to advisory packages in dnf5daemon
- Review and fix missing commands
- Document dnf5daemon advisory
- Document system-upgrade
- system-upgrade: offline status subcommand
- Add aliases `offline-distrosync`, `offline-upgrade`
- Add `system-upgrade --offline` option
- Add `offline`, `system-upgrade` commands

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
