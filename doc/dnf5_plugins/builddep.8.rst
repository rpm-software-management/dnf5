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

.. _builddep_plugin_ref-label:

#################
 Builddep Command
#################

Synopsis
========

``dnf5 builddep [options] [<package>...]``


Description
===========

Install missing dependencies for building an RPM package.

.. warning:: Build dependencies in a source package (i.e. src.rpm) might be different
             than you would expect because dependencies were evaluated according macros
             set on the package build host.


Options
=======

``--allowerasing``
    Allow erasing of installed packages to resolve dependencies resolution problems.

``--skip-unavailable``
    Allow skipping build dependencies not available in repositories. All available build dependencies will be installed.

``-D "macro expr", --define="macro expr"``
    Define a rpm macro. Set the value "expr" to the macro "macro" when parsing spec files. Does not apply for source rpm files.


Arguments
=========

``<package>``
    Either path to .src.rpm, .nosrc.rpm or .spec file or package available in a repository.


Examples
========

``dnf builddep foobar.spec``
  Install the needed build requirements, defined in the foobar.spec file.

``dnf builddep foobar-1.0-1.src.rpm``
  Install the needed build requirements, defined in the foobar-1.0-1.src.rpm file.

``dnf builddep foobar-1.0-1``
  Look up foobar-1.0-1 in enabled repositories and install build requirements for its source rpm.

``dnf builddep -D 'scl python27' python-foobar.spec``
  Install the needed build requirements for the python27 SCL version of python-foobar.
