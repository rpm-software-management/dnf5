###############################################
 Modifications to the public API in dnf 5.2.0.0
###############################################

This page lists the differences in the public API of `DNF5 5.2 <https://github.com/rpm-software-management/dnf5/releases/tag/5.2.0.0>`_ compared to the previous major version, DNF5 5.1.


Wrapping struct attributes
==========================

Public attributes in struct objects have been wrapped in getter and setter methods.

Before: ``bool ignore_case;`` Now: ``void set_ignore_case(bool); bool get_ignore_case() const;``

* ``libdnf5::GoalJobSettings``
* ``libdnf5::ResolveSpecSettings``
* ``libdnf5::repo::RepoCacheRemoveStatistics``
* ``libdnf5::rpm::Changelog``


Dropping deprecated methods
===========================

Methods that were previously marked as deprecated have been removed or moved to the private section of the class.

* ``libdnf5::create_file_logger(libdnf5::Base & base)`` (other variants still present)
* ``libdnf5::base::Base::load_config_from_file()``
* ``libdnf5::base::Base::with_config_file_path()``
* ``libdnf5::repo::Repo::add_libsolv_testcase()``
* ``libdnf5::repo::Repo::add_rpm_package()``
* ``libdnf5::repo::Repo::download_metadata()``
* ``libdnf5::repo::Repo::load()``
* ``libdnf5::repo::Repo::load_extra_system_repo()``
* ``libdnf5::repo::Repo::set_substitutions()``


Dropping unused or redundant items
==================================

Methods and objects that were unused or redundant have been removed.

* ``libdnf5::base::Base::get_comps()``
* ``libdnf5::comps::Comps``
* ``libdnf5::comps::EnvironmentSack``
* ``libdnf5::comps::GroupSack``
* ``libdnf5::repo::Repo::fresh()`` (use ``is_expired()`` instead)


Other changes
=============

* ``libdnf5::advisory::AdvisoryQuery``

  * ``filter_*`` methods have the default ``cmp_type`` ``IEXACT`` (was ``EQ`` before)

* ``libdnf5::repo::RepoSack``

  * ``update_and_load_enabled_repos()`` is deprecated now, ``load_repos()`` should be used instead

* ``libdnf5::LibraryVersion``

  * changing version format

    * Before: ``(major, minor, micro)`` Now: ``(prime, major, minor, micro)``
