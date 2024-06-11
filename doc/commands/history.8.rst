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

.. _history_command_ref-label:

################
 History Command
################

Synopsis
========

``dnf5 history <subcommand> [options] [<transaction-spec>]``


Description
===========

The ``history`` command in ``DNF5`` allows the user to view what has happened in past transactions
and offers several operations on these transactions, like undoing and redoing them. In order to
use the transactions in these commands, it is assumed that they were committed while using the
``history_record`` configuration option.


Subcommands
===========

``list``
    | List info about recorded transactions in the system.

``info``
    | Print details about specific transactions.

``undo``
    | Revert all actions from the specified transaction.

``redo``
    | Repeat the specified transaction.

``rollback``
    | Undo all transactions performed after the specified transaction.

``store``
    | Store the transaction into the file.

``replay``
    | Replay the transaction that was previously stored into the file.


Options for ``list`` and ``info``
=================================

``--reverse``
    | Reverse the order of transactions in the output.


Options for ``undo``
====================

``--skip-unavailable``
    | Allow skipping packages actions that are not possible perform.

``--ignore-extras``
    | Don't consider extra packages pulled into the transaction as errors.
    | They will still be reported as warnings.

``--ignore-installed``
    | Don't consider mismatches between installed and stored transaction packages as errors.
    | They will still be reported as warnings.
    | Using this option can result in an empty transaction.
    | For install actions skip already installed packages.
    | For upgrade actions skip groups or environments that are not installed.
    | For remove actions skip not installed packages/groups/environments.


Examples
========

``dnf5 history list``
    | List all transactions, where the most recent transaction is printed first.

``dnf5 history info 4``
    | Show detailed info about the fourth transaction.

``dnf5 history info last``
    | Show detailed info about the last transaction.

``dnf5 history info last-1``
    | Show detailed info about the second to last transaction.

``dnf5 history list 4..8``
    | List transactions with id in 4 to 8 range.

``dnf5 history undo last``
    | Undo the last transaction.

``dnf5 history undo 4 --ignore-extras``
    | Undo the fourth transaction ignoring extra packages pulled into the reverting transaction.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
