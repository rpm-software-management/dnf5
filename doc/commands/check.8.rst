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

.. _check_command_ref-label:

##############
 Check Command
##############

Synopsis
========

``dnf5 check [options]``


Description
===========

Checks the local packagedb and produces information on any problems it finds.
The set of checks performed can be specified with options.


Options
=======

``--dependencies``
    | Show missing dependencies and conflicts.

``--duplicates``
    | Show duplicated packages.

``--obsoleted``
    | Show obsoleted packages.


Examples
========

``dnf5 check``
    | Show all problems.

``dnf5 check --dependencies --obsoleted``
    | Show missing dependencies, conflicts and obsoleted packages.
