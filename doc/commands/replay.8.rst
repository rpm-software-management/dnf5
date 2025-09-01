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

.. _replay_command_ref-label:

###############
 Replay Command
###############

Synopsis
========

``dnf5 replay [options] <transaction-path>``


Description
===========

Replay a transaction stored in a directory at ``<transaction-path>``. The transaction directory can be created either by
the ``--store`` option, available for all transaction commands, or by `History Store Command`. The replay will perform
the exact same operations on the packages as in the original transaction and will return with an error in case of any
differences in installed packages or their versions.

To run the replay the transaction directory has to contain a file with the transaction in JSON format named ``transaction.json``.
The directory can also contain packages, comps groups or comps environments that will be used used in the replayed transaction.


Options
=======

``--ignore-extras``
    | Don't consider extra packages pulled into the transaction as errors.
    | They will still be reported as warnings.

``--ignore-installed``
    | Don't consider mismatches between installed and stored transaction packages as errors.
    | They will still be reported as warnings.
    | For install actions skip already installed packages.
    | For upgrade actions skip groups or environments that are not installed.
    | For remove actions skip not installed packages/groups/environments.
    | Using this option can result in an empty transaction.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.

``--skip-unavailable``
    | In case some packages stored in the transaction are not available on the target system,
    | skip them instead of erroring out.

Examples
========

``dnf5 replay ./transaction``
    | Replay a transaction stored at ./transaction.

``dnf5 replay ./transaction --skip-unavailable``
    | Replay a transaction stored at ./transaction skipping unavailable packages.
