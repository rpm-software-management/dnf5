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

..
    # TODO(jkolarik): Command not ready yet in upstream ...

.. _module_command_ref-label:

###############
 Module Command
###############

Synopsis
========

``dnf5 module <subcommand> [options] [<module-spec>]``


Description
===========


Subcommands
===========

``list``
    | List module streams.

``info``
    | Print details about module streams.

``provides``
    | Print module and module profile the specified packages come from.

``enable``
    | Enable module streams and make their packages available.

``disable``
    | Disable modules including all their streams.

``switch-to``
    | Enable different module streams, upgrade their profiles and distro-sync packages.

``reset``
    | Reset module state so it's no longer enabled or disabled.

``install``
    | Install module profiles, including their packages.

``remove``
    | Remove installed module profiles including their packages.


Options
=======


Examples
========

