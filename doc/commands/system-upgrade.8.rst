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

.. _system-upgrade_command_ref-label:

#######################
 System-Upgrade Command
#######################

Synopsis
========

``dnf5 system-upgrade <subcommand> [options]``


Description
===========

The ``system-upgrade`` command is used to upgrade the system to a new major release. First, the ``download`` subcommand downloads packages while the system is running normally. Then, the ``reboot`` subcommand reboots the system into a minimal "offline" environment to apply the upgrades.

``dnf5 system-upgrade`` is a recommended way to upgrade a system to a new major release. Before you proceed, ensure that your system is fully upgraded (``dnf5 --refresh upgrade``).

``system-upgrade`` shares many subcommands with the :ref:`offline subcommand <offline_command_ref-label>`.


Subcommands
===========

``clean``
    | See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`

``download``
    | Downloads all packages needed to upgrade to a new major release and checks that they can be installed.

``log``
    | See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`

``reboot``
    | See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`


Options
=======

``--releasever=<version>``
    | Required. The version to upgrade to. Sets ``$releasever`` in all enabled repos. Usually a number, or ``rawhide``.

``--no-downgrade``
    | Behave like ``dnf5 update``: do not install packages from the new release if they are older than what is currently installed. This is the opposite of the default behavior, which behaves like ``dnf5 distro-sync``, always installing packages from the new release, even if they are older than the currently-installed version.

``--number=<boot number>``
    | See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`

``--poweroff``
    | See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`


Examples
========

Typical upgrade usage
---------------------

``dnf5 --refresh upgrade``

``dnf5 system-upgrade download --releasever 40``

``dnf5 system-upgrade reboot``


Show logs from last upgrade attempt
-----------------------------------

``dnf5 system-upgrade log --number=-1``


See Also
========

    | :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`
    | https://www.freedesktop.org/wiki/Software/systemd/SystemUpdates
