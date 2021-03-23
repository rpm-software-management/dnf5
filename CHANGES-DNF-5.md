PackageSet::operator[]
----------------------
It was removed due to insufficient O(n^2) performance.
Use PackageSet iterator to access the data instead.


Package::get_epoch()
--------------------
The return type was changed from `unsigned long` to `std::string`.

dnf_sack_set_installonly, dnf_sack_get_installonly, dnf_sack_set_installonly_limit, dnf_sack_get_installonly_limit
------------------------------------------------------------------------------------------------------------------
Function were dropped as unneeded. Installonly packages are taken directly from main Conf in Base.

Query::filter() - HY_PKG_UPGRADES_BY_PRIORITY, HY_PKG_OBSOLETES_BY_PRIORITY, HY_PKG_LATEST_PER_ARCH_BY_PRIORITY
---------------------------------------------------------------------------------------------------------------
Priority filter was separate from other filter. 
Example: `HY_PKG_LATEST_PER_ARCH_BY_PRIORITY` was replaced by combination
`query.ifilter_priority().ifilter_latest()`

Query::filter() - HY_PKG_LATEST
-------------------------------
The filter is replaced by `ifilter_latest()` which have the same behaviour like `HY_PKG_LATEST_PER_ARCH`
