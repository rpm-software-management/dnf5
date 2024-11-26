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

.. _reposync_plugin_ref-label:

##################
 Reposync Command
##################

Synopsis
========

``dnf5 [GLOBAL OPTIONS] reposync [OPTIONS]``


Description
===========

The ``reposync`` command creates local copies of remote repositories. It avoids re-downloading packages that are already present in the local directory.

By default, ``reposync`` synchronizes all enabled repositories. However, you can customize the set of repositories to be synchronized using standard DNF5 options such as ``--repo``, ``--enable-repo``, or ``--disable-repo``.


Options
=======

``--arch=<architecture>, -a <architecture>``
    Download only packages for the specified architecture. This option can be specified multiple times. The default behavior is to download packages for all architectures.

``--delete``
    Remove local packages that are no longer present in the remote repository.

``--destdir=<path>``
    Specifies the root path where downloaded repositories are stored, relative
    to the current working directory. Defaults to the current working
    directory. Each downloaded repository will have a subdirectory named after
    its ID within this path.

``--download-metadata``
    Download all repository metadata. The downloaded copy is instantly usable
    as a repository without the need to run ``createrepo_c``. When used with
    ``--newest-only``, only the latest package versions are downloaded. However,
    the metadata will still reference also older packages. To avoid issues caused
    by missing RPM files, consider updating the metadata using ``createrepo_c --update``.
    Otherwise, DNF will encounter errors when attempting to install older packages.

``--gpgcheck, -g``
    Remove packages that fail PGP signature verification after downloading. The
    command exits with a code of ``1`` if at least one package is removed.

    Note: For repositories configured with ``gpgcheck=0``, PGP signatures are not
    checked, even when this option is used.

``--metadata-path=<path>``
    Specifies the root path where downloaded metadata files are stored. If not
    specified, it defaults to the value of ``--destdir``.

``--newest-only, -n``
    Download only the latest package versions from each repository.

``--norepopath``
    Prevents the repository id from being added to the download path. This
    option can only be used when syncing a single repository. (The default
    behavior adds the repository id to the path.)

``--remote-time``
    Attempts to set the timestamps of local downloaded files to match those on
    the remote side.

``--safe-write-path=<path>``
    Defines the filesystem path prefix where reposync is allowed to write
    files. If not specified, it defaults to the repository's download path.
    This option is useful for repositories that use relative locations of
    packages leading outside of the repository directory (e.g.,
    ``../packages_store/foo.rpm``).

    Caution: Any file under the ``safe-write-path`` can be overwritten. This option
    can only be used when syncing a single repository.

``--srpm``
    Downloads source packages. Equivalent to using ``--arch=src``.

``--urls, -u``
    Prints the URLs of the files that would be downloaded without actually
    downloading them.



Examples
========

``dnf reposync --repoid=the_repo``
    Synchronize all packages from the repository with id ``the_repo``. The
    synchronized copy is saved in ``the_repo`` subdirectory of the current
    working directory.

``dnf reposync --destdir=/my/repos/path --repoid=the_repo``
    Synchronize all packages from the repository with id ``the_repo``. In this
    case files are saved in ``/my/repos/path/the_repo`` directory.

``dnf reposync --repoid=the_repo --download-metadata``
    Synchronize all packages and metadata from ``the_repo`` repository.

    Repository synchronized with ``--download-metadata`` option can be directly
    used with DNF for example by using ``--repofrompath`` option:

    ``dnf --repofrompath=syncedrepo,the_repo --repoid=syncedrepo list --available``
