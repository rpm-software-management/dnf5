.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

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

``-D "macro expr", --define="macro expr"``
    | Define a rpm macro. Set the value "expr" to the macro "macro" when parsing spec files. Does not apply for source rpm files.

``--with=OPTION, --without=OPTION``
    | Enable or disable conditional build OPTION when parsing spec files. Does not apply for source rpm files.

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

``--skip-unavailable``
    | Allow skipping packages that are not possible to downgrade. All remaining packages will be downgraded.

``--allow-downgrade``
    | Enable downgrade of dependencies when resolving the requested operation.

``--no-allow-downgrade``
    | Disable downgrade of dependencies when resolving the requested operation.

.. include:: ../_shared/options/from-repo.rst

``--spec``
    | Treat command line arguments following this option as spec files.

``--srpm``
    | Treat command line arguments following this option as source rpms.

.. include:: ../_shared/options/transaction.rst


Arguments
=========

``<package>``
    | Either path to .src.rpm, .nosrc.rpm or .spec file or package available in a repository.


Examples
========

``dnf builddep foobar.spec``
    | Install the needed build requirements, defined in the foobar.spec file.

``dnf builddep foobar-1.0-1.src.rpm``
    | Install the needed build requirements, defined in the foobar-1.0-1.src.rpm file.

``dnf builddep foobar-1.0-1``
    | Look up foobar-1.0-1 in enabled repositories and install build requirements for its source rpm.

``dnf builddep -D 'scl python27' python-foobar.spec``
    | Install the needed build requirements for the python27 SCL version of python-foobar.

``dnf builddep --without=selinux foobar.spec``
    | Install the needed build requirements except those for optional SELinux support.
