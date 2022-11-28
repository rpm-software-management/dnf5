# [DNF5](https://github.com/rpm-software-management/dnf5) x [DNF4](https://github.com/rpm-software-management/dnf) differences

Changes on the API:
-------------------
### PackageSet::operator[]
It was removed due to insufficient O(n^2) performance.
Use PackageSet iterator to access the data instead.


### Package::get_epoch()
The return type was changed from `unsigned long` to `std::string`.


### DNF: Package.size, libdnf: dnf_package_get_size()
The return value was ambiguous, returning either package or install size.
Use Package::get_package_size() and Package::get_install_size() instead.


### dnf_sack_set_installonly, dnf_sack_get_installonly, dnf_sack_set_installonly_limit, dnf_sack_get_installonly_limit
The functions were dropped as unneeded. The installonly packages are taken directly from main Conf in Base.


### Query::filter() - HY_PKG_UPGRADES_BY_PRIORITY, HY_PKG_OBSOLETES_BY_PRIORITY, HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
The priority filter was separated into a standalone method.
Combine `query.filter_priority()` with `query.filter_latest_evr()` or another filter to achieve the original functionality.


### Query::filter() - HY_PKG_LATEST
The filter was replaced with `filter_latest_evr()` which has the same behavior as `HY_PKG_LATEST_PER_ARCH`


Changes on the command line:
----------------------------
dnf upgrade-minimal -> dnf upgrade --minimal

Commands cannot have optional subcommands and optional arguments. Is some cases subcommand can have the same string as
an argument. It means there is a difficulty to find advisory for package with the name `list`, `info`, or `summary`.
`dnf history info <transaction ID>` -> `dnf history --info <transaction ID>`
`dnf updateinfo info` -> `dnf updateinfo --info`

Options that cannot be applied to all command or can be applied but without any effect should be removed from general
options and implemented only for related commands
`--best`, `--nobest` are only related several transaction commands

Renaming boolean options to format `--<option>`, and `--no-<option>`
`--nobest` -> `--no-best`. Proposing to keep `--nobest` as a deprecated option.
