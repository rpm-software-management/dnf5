..
    Copyright Contributors to the libdnf project.

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

.. TODO(jkolarik): Add link to modularity in "What are the metadata types" when the page exists
.. TODO(jkolarik): Add link to dnf5.conf options when the page exists


.. _caching_misc_ref-label:

########
 Caching
########


Description
===========

This page aims to provide an overview of the various instruments, such as configuration options,
command-line parameters, and commands, available in DNF5 for manipulating cached data and
their associated scenarios.


Cached Data Location
====================

Following directory structure illustrates a typical DNF5 cache directory:

.. code-block:: bash
    :linenos:

    /var/cache/libdnf5/
    ├── fedora-*
    │   ├── metalink.xml
    │   ├── repodata
    │   │   ├── *-comps-Everything.x86_64.xml
    │   │   ├── *-primary.xml.zck
    │   │   └── repomd.xml
    │   └── solv
    │       ├── fedora-group.solvx
    │       └── fedora.solv
    ├── temporary_files.toml
    └── updates-*
        ├── metalink.xml
        ├── packages
        │   └── bash-5.2.21-1.fc38.x86_64.rpm
        ├── repodata
        │   ├── *-primary.xml.zck
        │   ├── *-updateinfo.xml.zck
        │   ├── *-comps-Everything.x86_64.xml.gz
        │   └── repomd.xml
        └── solv
            ├── updates-group.solvx
            ├── updates-updateinfo.solvx
            └── updates.solv

The default root cache directory is ``/var/cache/libdnf5``, but when DNF5 runs as another user,
it uses the cache from ``~/.cache/libdnf5`` with the same structure. The root cache
directory can be redefined using the ``system_cachedir`` configuration option, and the user
cachedir with the ``cachedir`` option.

Within the cache directory, there are subdirectories corresponding to each configured repository,
such as ``fedora-*`` and ``updates-*``. These contain metadata files in the ``repodata`` directory
and solver-generated cached files in the ``solv`` directory. The solver files, used to enhance
performance in resolving package dependencies or running queries, can be enabled or disabled on
a repository level through the ``build_cache`` configuration option. The ``packages`` directory
may store downloaded packages from a repository, and a ``metalink`` or ``mirrorlist`` file provides
information on the remote locations of the repository data.

Additionally, the root cache directory contains a ``temporary_files.toml`` file related to
temporarily stored packages in the system.


Metadata Types
==============

There are several types of metadata downloaded from remote locations and processed in DNF5.

Some metadata is mandatory and always considered. The main repository metadata file, ``repomd.xml``,
contains information about specific metadata type files related to a repository, such as checksums,
file sizes, and their locations in the metadata hierarchy. Another mandatory file is the ``primary``
metadata file, providing detailed information about available packages, including package names,
versions, dependencies, etc. If DNF5 is compiled with modularity support, ``modules`` metadata is
also downloaded and processed.

Other metadata types are optional and can be loaded into DNF5 in the following ways:

* Explicit user request by adding the requested type to the ``optional_metadata_types`` configuration option
* Automatically during runtime, depending on the CLI command used
* For ``filelists`` metadata, when the user passes any filepath argument

Here is the list of supported optional metadata types:

* ``comps``: Metadata containing package groups and environment descriptions
* ``filelists``: Information about all files provided by packages
* ``updateinfo``: Security-related updates and advisories information
* ``presto``: Information related to delta RPMs
* ``other``: Additional metadata, such as changelogs

If the required metadata is not present in the system, it can result in different scenarios,
such as returning an empty query, error output for no match for an argument, or an error when
resolving a transaction.

.. _caching_packages_ref-label:

Caching Packages
================

By default, DNF5 does not cache downloaded package data, as the ``keepcache`` option is configured
to ``False``. With this setting, every time DNF5 downloads a package from the remote location,
it tracks it within the ``temporary_files.toml`` in the repository cache directory. After the next
successful transaction run, all the files are removed based on this list. This process occurs only
when the transaction contains any inbound action; otherwise, packages are retained, considering
potential use cases.

When the keepcache option is set to ``True``, downloaded files are not tracked, and they could be
removed later, either manually or by executing the ``clean`` command, for example,
``dnf5 clean packages``.

Packages are always retained when downloaded using the ``download`` command.


Sharing Root Cache Among Users
==============================

Typically, DNF5 is run with superuser privileges to make system changes. However, there are
scenarios where executing queries without elevated privileges is sufficient. In such cases,
we may need to download the entire repository metadata for the user account from the remote,
even if there is existing data in the root's cache.

In DNF5, when checking for empty, expired, or invalid repository metadata while running under
a non-root account, we first examine the root's location. If metadata is present there, it is
copied to the user cache location. Note that this cloning of metadata is optimized when
copy-on-write functionality is present on the filesystem, such as with btrfs.


Cacheonly Option vs Parameter
=============================

To instruct DNF5 to operate exclusively with cached data, avoiding downloads from remote locations,
two instruments are available. First, using the ``cacheonly`` configuration option, we can specify
either ``metadata`` to utilize only repository metadata from the cache or ``all`` to include
the entire cache, disallowing any package downloads. Alternatively, when the ``--cacheonly``
parameter is employed, it automatically sets the configuration option to ``all``, resulting
in a fully cache-driven operation.

Using the ``metadata`` value for the ``cacheonly`` configuration can be advantageous when optional
repositories are temporarily unavailable or when we know they are unnecessary for our current
use case. It is also useful when cached metadata is not the latest or has expired but is still
functional.


Delete, Create, Update
======================

The following commands are used for manual cache deletion and creation:

    | :manpage:`dnf5-clean(8)`, :ref:`Clean command <clean_command_ref-label>`
    | :manpage:`dnf5-makecache(8)`, :ref:`Makecache command <makecache_command_ref-label>`

To force metadata update before executing a command, use the ``--refresh`` parameter.
