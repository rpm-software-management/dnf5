.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

..
    # TODO(jkolarik): Still some subcommands missing in the upstream (install, ...)

.. _environment_command_ref-label:

####################
 Environment Command
####################

Synopsis
========

``dnf5 environment <subcommand> [options] [<environment-spec>...]``


Description
===========

The ``environment`` command in ``DNF5`` offers several queries for getting information
about environments and groups related to them. You can install environments
with the ``install`` command as ``install @environment-id``.

Optional ``environment-spec`` arguments could be passed to filter only environments with given names.


Subcommands
===========

``list``
    | List available environments.

``info``
    | Print details about environments.


Options
=======

``--available``
    | Show only available environments. Those which are not installed, but known to ``DNF5``.

``--installed``
    | Show only installed environments.


Examples
========

``dnf5 environment list``
    | Show list of all environments.

``dnf5 environment info "KDE Plasma Workspaces"``
    | Show detailed info about the ``KDE`` environment.


See Also
========

    | :manpage:`dnf5-comps(7)`, :ref:`Comps groups and environments <comps_misc_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
