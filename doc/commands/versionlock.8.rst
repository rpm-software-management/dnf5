.. Copyright Contributors to the DNF5 project.
..
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

.. _versionlock_command_ref-label:

####################
 Versionlock Command
####################

Synopsis
========

``dnf5 versionlock <subcommand> <package-spec-N>...``


Description
===========

The ``versionlock`` command in ``DNF5`` takes a set of names and versions for
packages and excludes all other versions of those packages. This allows you to
protect packages from being updated by newer versions. Alternately, it accepts
a specific package version to exclude from updates, e.g. for when it's
necessary to skip a specific release of a package that has known issues.

The plugin will walk each entry of the versionlock file, and exclude any
package of given name that doesn't match conditions listed within the file.
This is basically the same as using `dnf5 --exclude` for the package name itself
(as you cannot exclude installed packages), but dnf will still see the versions
you have installed/versionlocked as available so that `dnf reinstall` will
still work.

Note the versionlock command does not apply any excludes in non-transactional
operations like `repoquery`, `list`, `info`, etc.


Subcommands
===========

``add``
    | Add a versionlock for all available packages matching the spec. It means that only versions of packages represented by ``package-spec-N`` are available for transaction operations. The NEVRAs to lock to are first searched among installed packages and then (if none is found) in all currently available packages.

``exclude``
    | Add an exclude (within versionlock) for the available packages matching the spec. It means that packages represented by ``package-spec-N`` will be excluded from transaction operations.

``clear``
    | Remove all versionlock entries.

``delete``
    | Remove any matching versionlock entries.

``list``
    | List the current versionlock entries.


Examples
========

``dnf5 versionlock add acpi``
    | If acpi package is installed, lock it to currently installed version. If it's not installed, lock acpi to any of currently available versions.

``dnf5 versionlock list``
    | Show current versionlock configuration.

``dnf5 versionlock delete acpi``
    | Remove any rules for acpi package.

``dnf5 versionlock exclude iftop-1.2.3-7.fc38``
    | Exclude iftop-1.2.3-7.fc38 release.


Versionlock file format
=======================

The versionlock file is a TOML file stored in location `/etc/dnf/versionlock.toml`.
The file must contain the `version` key, currently supported version is `1.0`.
Then it contains `packages` - a list of locking entries. Each entry consist of the package name and a list of conditions. The package name specification supports the same glob pattern matching as the shell. All the conditions must be true for a package to match (they are combined using logical AND). All entries are then combined together using logical OR operation.


Example of versionlock file
---------------------------


.. code-block:: toml

    version = "1.0"

    # keep package bash on version 0:5.2.15-5.fc39
    [[packages]]
    name = "bash"               # name of the package
    comment = "description"     # optional description of the entry
    [[packages.conditions]]     # conditions for the package "bash"
    key = "evr"                 # epoch, evr, and arch keys are supported
    comparator = "="            # <, <=, =, >=, >, and != operators are supported
    value = "0:5.2.15-5.fc39"   # pattern to match

    # exclude iftop-1.2.3-7.fc38 version (versionlock exclude iftop-1.2.3-7.fc38)
    [[packages]]
    name = "iftop"
    [[packages.conditions]]
    key = "evr"
    comparator = "!="
    value = "0:1.0-0.31.pre4.fc39"

    # keep acpi on major version 3
    [[packages]]
    name = "acpi"
    [[packages.conditions]]
    key = "evr"
    comparator = "<"
    value = "4"
    [[packages.conditions]]
    key = "evr"
    comparator = ">="
    value = "3"

    # keep all *nvidia* packages on version 3:570.*
    [[packages]]
    name = "*nvidia*"
    [[packages.conditions]]
    key = "evr"
    comparator = ">="
    value = "3:570"
    [[packages.conditions]]
    key = "evr"
    comparator = "<"
    value = "3:571"


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
