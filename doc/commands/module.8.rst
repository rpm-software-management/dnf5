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

.. _module_command_ref-label:

###############
 Module Command
###############

Synopsis
========

``dnf5 module <subcommand> [options] [<module-spec>...]``


Description
===========

Modularity is an alternative way of building, organizing and delivering packages.

Currently, only basic support is available for managing the modules,
as they are no longer supported in mainstream RPM distributions.

For more details see: https://docs.pagure.org/modularity/.


Subcommands
===========

``list``
    | List module streams. ``--enabled`` and ``--disabled`` options narrow down the requested list.

``info``
    | Print details about module streams.

``enable``
    | Enable module streams and make their packages available.

``disable``
    | Disable modules including all their streams.

``reset``
    | Reset module state so it's no longer enabled or disabled.


Options for ``list`` and ``info``
=================================

``--enabled``
    | Show only enabled modules.

``--disabled``
    | Show only disabled modules.


Options for ``enable``, ``disable``, ``reset``
==============================================

``--skip-broken``
    | Resolve any dependency problems by removing items that are causing problems from the transaction.
    | Used with ``enable`` command.

``--skip-unavailable``
    | Allow skipping modules that are not available in repositories.
    | All remaining items will be processed.


Examples
========

``dnf5 module list``
    | List all module streams available for your system.

``dnf5 module enable nodejs:8``
    | Make packages from the Node.js 8 stream available for installation.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
