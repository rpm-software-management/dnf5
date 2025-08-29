.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _repoclosure_plugin_ref-label:

####################
 Repoclosure Command
####################

Synopsis
========

``dnf5 repoclosure [options] [<pkg-spec>...]``


Description
===========

The ``repoclosure`` command allows you to analyze package metadata from multiple repositories. It checks all dependencies of the packages and provides a list of packages that have unresolved dependencies.

By default, ``repoclosure`` considers all enabled repositories when checking dependencies. However, you can customize the set of repositories by using standard DNF5 options such as ``--repo``, ``--enable-repo``, or ``--disable-repo``.


Options
=======

``--arch <arch>``
    | Query only packages for specified architecture, can be specified multiple times (default is all compatible architectures with your system).

``--best``
    | Check only the newest packages per arch.

``--check=REPO_ID,...``
    | Specify repositories to check, can be specified multiple times (default is all enabled repositories).
    | Accepted values are repository ids, or a glob of ids.

``--newest``
    | Check only the newest packages in the repos.

``<pkg-spec>``
    | Check closure for this package only.


Examples
========

``dnf5 repoclosure``
    | Display a list of unresolved dependencies for all enabled repositories.

``dnf5 repoclosure --repo rawhide --arch noarch --arch x86_64``
    | Display a list of unresolved dependencies for rawhide repository and packages with architecture noarch and x86_64.

``dnf5 repoclosure --repo rawhide zmap``
    | Display a list of unresolved dependencies for zmap package from rawhide repository.

``dnf5 repoclosure --repo rawhide --check myrepo``
    | Display a list of unresolved dependencies for myrepo, an add-on for the rawhide repository.
