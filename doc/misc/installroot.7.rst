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

.. _installroot_misc_ref-label:

######################
 Installroot Parameter
######################

Description
===========

The ``--installroot`` parameter is used to specify an alternative installroot,
relative to where all packages will be installed. Think of it like doing
``chroot <root> dnf``, except using ``--installroot`` allows ``DNF5`` to work
before the chroot is created.

`cachedir`, `system_cachedir`, `log` files, `releasever`, and `gpgkey` are
taken from or stored in the installroot. OpenPGP keys are imported into the
installroot from a path relative to the host which can be specified in the
repository section of configuration files.

`configuration` file, `reposdir`, and `vars` are taken from inside the
installroot, unless the command-line argument ``--use-host-config`` is
passed, in which case the configuration and environment from the host system
will be used.

Note: When a path is specified within a command line argument
(``--config=CONFIG_FILE_PATH`` in case of `configuration` file,
``--setopt=reposdir=/path/to/repodir`` for `reposdir`,
``--setopt=cachedir=/path/to/cachedir`` for `cachedir`,
``--setopt=system_cachedir=/path/to/system_cachedir`` for `system_cachedir`,
``--setopt=logdir=/path/to/logdir`` for `logdir`, or
``--setopt=varsdir=/paths/to/varsdir`` for `vars`), then this path is always
relative to the host with no exceptions. `pluginpath` and `pluginconfpath` are
relative to the host.

Note: You may also want to use the command-line option ``--releasever=RELEASEVER`` when creating
the installroot, otherwise the $releasever value is taken from the rpmdb within the installroot
(and thus it is empty at the time of creation and the transaction will fail). If ``--use-host-config``
is used, the releasever will be detected from the host (/) system. The new installroot path at the
time of creation does not contain the repository, releasever and dnf.conf files.

On a modular system you may also want to use the ``--setopt=module_platform_id=<module_platform_name:stream>``
command-line option when creating the installroot, otherwise the ``module_platform_id`` value will be
taken from the ``/etc/os-release`` file within the installroot (and thus it will be empty at the time of
creation, the modular dependency could be unsatisfied and modules content could be excluded).


Examples
========

``dnf5 --installroot=INSTALLROOT --releasever=RELEASEVER install system-release``
    Permanently sets the ``releasever`` of the system in the ``INSTALLROOT`` directory
    to ``RELEASEVER``.

``dnf5 --installroot=INSTALLROOT --setopt=reposdir=PATH --config /path/dnf.conf upgrade``
    Upgrades packages inside the installroot from a repository described by ``--setopt``
    using configuration from ``/path/dnf.conf``.
