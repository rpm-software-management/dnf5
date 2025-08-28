.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _repo_command_ref-label:

#############
 Repo Command
#############

Synopsis
========

``dnf5 repo <subcommand> [options] [<repo-spec>...]``


Description
===========

The ``repo`` command in ``DNF5`` offers several queries for getting information
about repositories configured on the system.


Subcommands
===========

``list``
    | List available repositories.

``info``
    | Show detailed info about repositories.


Options
=======

``--all``
    | Show information about all known repositories.

``--enabled``
    | Show information only about enabled repositories.
    | This is the default behavior.

``--disabled``
    | Show information only about disabled repositories.


Examples
========

``dnf5 repo info --all``
    | Print detailed info about all known repositories.

``dnf5 repo list --disabled *-debuginfo``
    | Print disabled repositories related to debugging.

``dnf5 config-manager setopt repo_id.enabled=0``
    | Persistently disable repository using the config-manager plugin command.
    | See :manpage:`dnf5-config-manager(8)` for more details.
