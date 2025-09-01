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

.. _repoquery_command_ref-label:

##################
 Repoquery Command
##################

Synopsis
========

``dnf5 repoquery [options] [<package-spec-NIF>...]``


Description
===========

The ``repoquery`` command in ``DNF5`` is used for querying packages by matching
various input criteria from the user. Arguments defined in ``spec`` list are used
as ``<package-file-spec>``.


Options
=======

.. include:: ../_shared/options/advisories.rst

.. include:: ../_shared/options/advisory-severities.rst

``--arch=ARCH,...``
    | Limit to packages of these architectures.
    | This is a list option.

``--available``
    | Query available packages.
    | This is the default behavior.
    | Can be combined with ``--installed`` to query both installed and available packages.

.. include:: ../_shared/options/bugfix.rst

.. include:: ../_shared/options/bzs.rst

.. include:: ../_shared/options/cves.rst

``--disable-modular-filtering``
    | Include packages of inactive module streams.

``--duplicates``
    | Limit to installed duplicate packages (i.e. more package versions for  the  same  name and architecture).
    | Installonly packages are excluded from this set.

.. include:: ../_shared/options/enhancement.rst

``--exactdeps``
    | Limit to packages that require <capability> specified by --whatrequires or --whatdepends.
    | This option is stackable with --whatrequires or --whatdepends only.

``--extras``
    | Limit to installed packages that are not present in any available repository.

``-f, --file=FILE,...``
    | Limit to packages that own these files.
    | This is a list option.

``--installed``
    | Query installed packages.
    | Can be combined with ``--available`` to query both installed and available packages.

.. include:: ../_shared/options/installed-from-repo.rst

``--installonly``
    | Limit to installed installonly packages.

``--latest-limit=N``
    | Limit to N latest packages for a given name.arch (or all except N latest if N is negative).

``--leaves``
    | Limit to groups of installed packages not required by other installed packages.

.. include:: ../_shared/options/newpackage.rst

``--providers-of=PACKAGE_ATTRIBUTE``
    | After filtering is finished get selected attribute of packages and output packages that provide it.
    | The outputted packages are limited by ``--available``, ``--installed`` and ``--arch`` options.
    | Supports: conflicts, depends, enhances, obsoletes, provides, recommends, requires, requires_pre, suggests, supplements.

``--recent``
    | Limit to only recently changed packages.

``--recursive``
    | This option is stackable with --whatrequires or --providers-of=requires only.
    | When used with --whatrequires: it extends the output with packages that require anything provided by outputted packages.
    | When used with --providers-of=requires: it extends the output with packages that provide anything required by outputted packages.
    | It repeats the output extension as long as new packages are being added.
    | The added packages are limited by ``--available``, ``--installed`` and ``--arch`` options.

.. include:: ../_shared/options/security.rst

``--srpm``
    | After filtering is finished use packages' corresponding source RPMs for output.
    | Enables source repositories.

``--unneeded``
    | Limit to unneeded installed packages (i.e. packages that were installed as dependencies but are no longer needed).
    | This switch lists packages that are going to be removed after executing the `autoremove` command.

``--upgrades``
    | Limit to available packages that provide an upgrade for some already installed package.

``--userinstalled``
    | Limit to packages that are not installed as dependencies or weak dependencies.
    | This means limit to packages that were installed at the user request or indirectly as a part of a module profile or comps group. Additionally it returns packages with unknown reason.
    | The result may be influenced by the "exclude" option in the configuration file.
    | To get an exact reason of the installation, use ``--queryformat '%{name} %{reason}\n'`` option.

``--whatconflicts=CAPABILITY,...``
    | Limit to packages that conflict with any of <capabilities>.
    | This is a list option.

``--whatdepends=CAPABILITY,...``
    | Limit to packages that require, enhance, recommend, suggest or supplement any of <capabilities>.
    | This is a list option.

``--whatenhances=CAPABILITY,...``
    | Limit to packages that enhance any of <capabilities>. Use --whatdepends if you want to list all depending packages.
    | This is a list option.

``--whatobsoletes=CAPABILITY,...``
    | Limit to packages that obsolete any of <capabilities>.
    | This is a list option.

``--whatprovides=CAPABILITY,...``
    | Limit to packages that provide any of <capabilities>. Capabilities :ref:`specifying a file provide <file_provides_ref-label>` are also matched against file provides.
    | This is a list option.

``--whatrecommends=CAPABILITY,...``
    | Limit to packages that recommend any of <capabilities>. Use --whatdepends if you want to list all depending packages.
    | This is a list option.

``--whatrequires=CAPABILITY,...``
    | Limit to packages that require any of <capabilities>. Use --whatdepends if you want to list all depending packages.
    | This is a list option.

``--whatsuggests=CAPABILITY,...``
    | Limit to packages that suggest any of <capabilities>. Use --whatdepends if you want to list all depending packages.
    | This is a list option.

``--whatsupplements=CAPABILITY,...``
    | Limit to packages that supplement any of <capabilities>. Use --whatdepends if you want to list all depending packages.
    | This is a list option.

Formatting Options
==================

Set what information is displayed about each package. The following are mutually exclusive, i.e. at most one can be specified. If no formatting option is given, selected packages are displayed in ``"%{full_nevra}"`` queryformat.

``--conflicts``
    | Like ``--qf "%{conflicts}"`` but deduplicated and sorted per line.

``--depends``
    | Like ``--qf "%{depends}"`` but deduplicated and sorted per line.

``--enhances``
    | Like ``--qf "%{enhances}"`` but deduplicated and sorted per line.

``--files``
    | Like ``--qf "%{files}"`` but deduplicated and sorted per line.

``--obsoletes``
    | Like ``--qf "%{obsoletes}"`` but deduplicated and sorted per line.

``--provides``
    | Like ``--qf "%{provides}"`` but deduplicated and sorted per line.

``--recommends``
    | Like ``--qf "%{recommends}"`` but deduplicated and sorted per line.

``--requires``
    | Like ``--qf "%{requires}"`` but deduplicated and sorted per line.

``--requires-pre``
    | Like ``--qf "%{requires_pre}"`` but deduplicated and sorted per line.

``--sourcerpm``
    | Like ``--qf "%{sourcerpm}"`` but deduplicated and sorted per line.

``--suggests``
    | Like ``--qf "%{suggests}"`` but deduplicated and sorted per line.

``--supplements``
    | Like ``--qf "%{supplements}"`` but deduplicated and sorted per line.

``--location``
    | Like ``--qf "%{location}"`` but deduplicated and sorted per line.

``-i, --info``
    | Show detailed information about the package.

``--changelogs``
    | Print the package changelogs.

``--querytags``
    | Display available tags for --queryformat.

``--queryformat=<format>``
    | Display format for packages. The ``<format>`` string can contain tags (``%{<tag>}``) which are replaced with corresponding attributes of the package.
    | Default is ``"%{full_nevra}"``. The ``<format>`` string is expanded and deduplicated for each package.
    |
    | * ``arch`` - Display architecture of the package.
    | * ``buildtime`` - Display buildtime of the package in Unix time.
    | * ``conflicts`` - Display capabilities that the package conflicts with. Separated by new lines.
    | * ``debug_name`` - Display name of debuginfo package of the package.
    | * ``depends`` - Display capabilities that the package depends on, enhances, recommends, suggests or supplements. Separated by new lines.
    | * ``description`` - Display description of the package.
    | * ``downloadsize`` - Display download size of the package.
    | * ``enhances`` - Display capabilities enhanced by the package. Separated by new lines.
    | * ``epoch`` - Display epoch of the package.
    | * ``evr`` - Display epoch:version-release of the package. Epoch 0 is omitted.
    | * ``files`` - Show files in the package. Separated by new lines.
    | * ``from_repo`` - Display id of repository the package is installed from. Empty for not installed packages.
    | * ``full_nevra`` - Display name-epoch:version-release.arch of the package. Even epoch 0 is included.
    | * ``group`` - Display group of the package. This is not Comps group.
    | * ``location`` - Display location of the package.
    | * ``installsize`` - Display install size of the package.
    | * ``installtime`` - Display install time of the package.
    | * ``license`` - Display license of the package.
    | * ``name`` - Display name of the package.
    | * ``obsoletes`` - Display capabilities obsoleted by the package. Separated by new lines.
    | * ``packager`` - Display packager of the package.
    | * ``prereq_ignoreinst`` - Display safe to remove requires_pre requirements of an installed package. Empty for not installed packages. Separated by new lines.
    | * ``provides`` - Display capabilities provided by the package. Separated by new lines.
    | * ``reason`` - Display reason why the packages was installed.
    | * ``recommends`` - Display capabilities recommended by the package. Separated by new lines.
    | * ``regular_requires`` - Display capabilities required by the package without its ``%pre``, ``%post``, ``%preun`` and ``%postun`` requirements. Separated by new lines.
    | * ``release`` - Display release of the package.
    | * ``repoid`` - Display id of repository the package is in.
    | * ``reponame`` - Display name of repository the package is in.
    | * ``requires`` - Display capabilities required by the package (combines regular_requires and requires_pre).
    | * ``requires_pre`` - For an installed package display capabilities that it depends on to run its ``%pre``, ``%post``, ``%preun`` and ``%postun`` scripts. For not installed package display just ``%pre`` and ``$post`` requirements. Separated by new lines.
    | * ``source_debug_name`` - Display name of debuginfo package for source package of the package.
    | * ``source_name`` - Display source RPM name of the package.
    | * ``sourcerpm`` - Display source RPM of the package.
    | * ``suggests`` - Display capabilities suggested by the package. Separated by new lines.
    | * ``summary`` - Display summary of the package.
    | * ``supplements`` - Display capabilities supplemented by the package. Separated by new lines.
    | * ``url`` - Display url of the package.
    | * ``vendor`` - Display vendor of the package.
    | * ``version`` - Display version of the package.
    |
    | The ``<format>`` string can also contain ``\n`` which will be replaced with a newline character on the output.

Examples
========

``dnf5 repoquery /etc/koji.conf``
    | List packages which provide the given file.

``dnf5 repoquery *http*``
    | List packages containing the ``http`` inside their name.

``dnf5 repoquery --installed --security``
    | List installed packages included in any security advisories.


See Also
========

    | :manpage:`dnf5-advisory(8)`, :ref:`Advisory command <advisory_command_ref-label>`
    | :manpage:`dnf5-leaves(8)`, :ref:`Leaves command <leaves_command_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
