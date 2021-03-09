PackageSet::operator[]
----------------------
It was removed due to insufficient O(n^2) performance.
Use PackageSet iterator to access the data instead.


Package::get_epoch()
--------------------
The return type was changed from `unsigned long` to `std::string`.

dnf_sack_set_installonly, dnf_sack_get_installonly 
--------------------------------------------------
Function were dropped as unneeded. Installonly packages are taken directly from main Conf in Base.
