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

.. _repomanage_plugin_ref-label:

###################
 Repomanage Command
###################

Synopsis
========

``dnf5 [GLOBAL OPTIONS] repomanage [OPTIONS] <path>``


Description
===========

`repomanage` prints newest or older packages in a repository specified by `<path>` for easy piping to xargs or similar programs.
In case `<path>` doesn't contain `repodata/repomd.xml`, it is searched for rpm packages which are then used instead.
If the repodata are present, `repomanage` uses them as the source of truth, it doesn't verify that they match the present rpm packages. In fact, `repomanage` can run with just the repodata, no rpm packages are needed.

If `<path>` specifies remote url, `repomanage` attempts to download the remote repodata into a temporary directory and use them.

Options
=======

``--new``
    | Print N newest packages for each name.arch (default).

``--old``
    | Print all packages except the N newest for each name.arch

``-k <N>``, ``--keep <N>``
    | Set package count N for --new and --old (default: 1)

``-s``, ``--space``
    | Print packages separated by spaces instead of new lines


Examples
========

``dnf repomanage --new .``
    | Display newest packages in current repository (directory).

``dnf repomanage --new --keep 2 ~/``
    | Display 2 newest versions of each package in "home" directory.

``dnf repomanage --old --space .``
    | Display all but the newest packages separated by space in current repository (directory).

``dnf repomanage http://download.example/repo/``
    | Download the repository metadata into a temporary directory, load them and display the newest packages.
