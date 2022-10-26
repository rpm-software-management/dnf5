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

.. _remove_command_ref-label:

###############
 Remove Command
###############

Synopsis
========

``dnf5 remove [options] <package-spec>...``


Description
===========

The ``remove`` command in ``DNF5`` is used for removing installed packages from the system.
If you want to remove also all dependencies that are no longer needed by any other package,
set the ``clean_requirements_on_remove`` configuration parameter to ``True``.


Examples
========

``dnf5 remove tito``
    | Remove the ``tito`` package.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`

