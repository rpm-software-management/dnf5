..
    Copyright Contributors to the libdnf project.
    SPDX-License-Identifier: GPL-2.0-or-later

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

.. _clean_command_ref-label:

##############
 Clean Command
##############

Synopsis
========

``dnf5 clean [options] <cache_types>...``


Description
===========

The ``clean`` command in ``DNF5`` is used to delete temporarily kept repository metadata
or marking the cache expired.

Arguments in ``cache_types`` specify which types of the cached data to cleanup:

`all`
    | Delete all temporary repository data from the system.

`packages`
    | Delete any cached packages.

`metadata`
    | Delete repository metadata.
    | This will delete the files which ``DNF5`` uses to determine the remote availability of packages.
    | Using this option will make ``DNF5`` download all the metadata the next time it is run.

`dbcache`
    | Delete cache files generated from the repository metadata.
    | This forces ``DNF5`` to regenerate the cache files the next time it is run.

`expire-cache`
    | Mark the repository metadata expired.
    | This forces ``DNF5`` to check the validity of the cache the next time it is run.


Examples
========

``dnf5 clean all``
    | Cleanup all repository cached data.

``dnf5 clean packages dbcache``
    | Cleanup all cached packages and dbcache metadata.
