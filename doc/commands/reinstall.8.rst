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

.. _reinstall_command_ref-label:

##################
 Reinstall Command
##################

Synopsis
========

``dnf5 reinstall [global options] <package-spec>...``


Description
===========

The ``reinstall`` command in ``DNF5`` is used for reinstalling packages defined in
``package-spec`` arguments.


Options
=======

``--allowerasing``
    | Allow erasing of installed packages to resolve any potential dependency problems.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.

``--skip-unavailable``
    | Allow skipping build dependencies not available in repositories. All available build dependencies will be installed.


Examples
========

``dnf5 reinstall tito``
    | Reinstall the ``tito`` package.

``dnf5 reinstall ~/Downloads/tito-0.6.21-1.fc36.noarch.rpm``
    | Reinstall the ``tito`` package from the local rpm file.
    | This can be useful f.e. when the given package is not available in any enabled repository.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
