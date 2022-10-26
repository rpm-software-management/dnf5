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


Options
=======

``--reverse``
    | Reverse the order of transactions in the output.


Examples
========


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`

