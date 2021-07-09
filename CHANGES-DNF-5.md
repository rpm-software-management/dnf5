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
Use Package::get_package_size() and Package::get_install_size() instead.


dnf_sack_set_installonly, dnf_sack_get_installonly, dnf_sack_set_installonly_limit, dnf_sack_get_installonly_limit
------------------------------------------------------------------------------------------------------------------
The functions were dropped as unneeded. The installonly packages are taken directly from main Conf in Base.


Query::filter() - HY_PKG_UPGRADES_BY_PRIORITY, HY_PKG_OBSOLETES_BY_PRIORITY, HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
---------------------------------------------------------------------------------------------------------------
The priority filter was separated into a standalone method.
Combine `query.filter_priority()` with `query.filter_latest_evr()` or another filter to achieve the original functionality.


Query::filter() - HY_PKG_LATEST
-------------------------------
The filter was replaced with `filter_latest_evr()` which has the same behavior as `HY_PKG_LATEST_PER_ARCH`
