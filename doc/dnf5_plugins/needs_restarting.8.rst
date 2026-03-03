..
    Copyright Contributors to the DNF5 project.
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

.. _needs_restarting_plugin_ref-label:

########################
needs-restarting Command
########################

Synopsis
========

``dnf5 needs-restarting [-p | --processes [-e | --exclude-services]] [-s | --services] [-r | --reboothint] [--json]``


Description
===========

The ``needs-restarting`` command determines whether the system should be rebooted to fully apply changes from package installations and upgrades. Without any options, ``dnf5 needs-restarting`` will report whether any important packages were installed or upgraded since boot. This set of important packages includes the kernel, systemd, each package listed here: https://access.redhat.com/solutions/27943, and any package marked with a ``reboot_suggested`` advisory.

The ``needs-restarting`` command will exit with code 1 if a reboot is recommended, or, when invoked with ``--services``, if any systemd service needs restarting. If no action is recommended, ``needs-restarting`` will exit with code 0.


Options
=======

``-p, --processes``
    | List processes that need restarting. If the package that provides the executable running, or any of its dependencies, have been updated since the process started, then restarting the process will be recommended.

``-e, --exclude-services``
    | Used with the ``-p, --processes`` option and will filter out any processes that are handled by systemd services.

``-s, --services``
    | List systemd services that need restarting. If the package that provides the service, or any of its dependencies, have been updated since the service started, then restarting the service will be recommended. Note that this approach is quite aggressive to recommend a restart when one may not be strictly necessary.

``-r, --reboothint``
    | Has no effect, kept for compatibility with DNF 4. "dnf4 needs-restarting -r" provides the same functionality as "dnf5 needs-restarting".

``--json``
    | Request JSON output format for machine-readable results.

JSON Output
===========

* ``dnf5 needs-restarting --json``

The command returns a JSON array containing a single object with the reboot hint information. The object contains the following fields:

    - ``type`` (string) - Type of output, always "reboot".
    - ``reboot_required`` (boolean) - Whether a reboot is required.
    - ``packages`` (array of strings) - List of packages that were updated since boot-up.
    - ``documentation`` (string) - Link to documentation explaining the reboot requirement.

* ``dnf5 needs-restarting --services --json``

The command returns a JSON array of objects, each describing a systemd service that needs restarting. Each object contains the following fields:

    - ``type`` (string) - Type of output, always "unit".
    - ``unit`` (string) - Name of the systemd service.

* ``dnf5 needs-restarting --processes --json``

The command returns a JSON array of objects, each describing a running process that needs restarting. Each object contains the following fields:

    - ``type`` (string) - Type of output, always "process".
    - ``pid`` (integer) - Process ID.
    - ``cmdline`` (array of strings) - Command line of the process.
    - ``package`` (string) - Package providing the executable.

The ``--json`` option can be combined with ``--exclude-services`` when using ``--processes`` to filter out processes managed by systemd services (e.g. ``dnf5 needs-restarting --processes --exclude-services --json``).

For empty results for services or processes, the commands return ``[]``. For reboot hints, even if no reboot is recommended, the command returns a JSON array containing a single object with ``reboot_required`` set to false, an empty array of ``packages``, and the ``documentation`` link.
