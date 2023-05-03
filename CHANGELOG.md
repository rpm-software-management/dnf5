# 5.0.10


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
