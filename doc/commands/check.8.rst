.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _check_command_ref-label:

##############
 Check Command
##############

Synopsis
========

``dnf5 check [options]``


Description
===========

Checks the local packagedb and produces information on any problems it finds.
The set of checks performed can be specified with options.


Options
=======

``--dependencies``
    | Show missing dependencies and conflicts.

``--duplicates``
    | Show duplicated packages.

``--obsoleted``
    | Show obsoleted packages.


Examples
========

``dnf5 check``
    | Show all problems.

``dnf5 check --dependencies --obsoleted``
    | Show missing dependencies, conflicts and obsoleted packages.
