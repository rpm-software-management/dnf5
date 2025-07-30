.. _migrating_ref-label:

############################
 Migrating from DNF to DNF5
############################

This chapter describes the changes that occurred on the system during
the migration from `DNF
<https://github.com/rpm-software-management/dnf>`_ to `DNF5
<https://github.com/rpm-software-management/dnf5>`_.  A number of
Fedora releases were used to migrate the system from dnf4 to dnf5.
Consider this chapter a historical account of what occurred rather
than steps you can take to migrate a dnf4 system to dnf5.

.. _introducing_microdnf_ref-label:

Introducing microdnf
====================

DNF was implemented in Python which means that a DNF-based system
needs to have the Python stack installed and required modules.  In
most cases this is fine, but there are instances where people want an
installed system that does not have Python at all.  This presents a
problem as the tool we provide to keep the system software updated
requires Python.

The request to have a Pythonless system had always been around, but no
one was really interested in coming up with a solution.  Then
containers showed up.  We already had virtual machines and
orchestration and now we have containers which are like chroot jails
with a better set of management tools.  Users want to use containers
to isolate individual applications and services and in those
containers they really do not want any unnecessary software.  That
includes Python.  At first the response was to simply rebuild your
container from scratch if system software needs updating, but users
still wanted the ability to update a container on the fly.  Now the
Pythonless system request has higher priority.

The microdnf project was started to provide a dnf command that had
compatible syntax, but was limited in functionality.  It was written
in C so it did not have to depend on Python, but that also meant it
could not use dnf plugins and it also meant that a lot of advanced
functionality simply was not implemented.  The goal was to provide a
dnf command for containers that could install, remove, and update
packages all without requiring Python.

There is an obvious downside to this approach.  The developers now
have two dnf implementations to maintain.  And the promise has been
made that microdnf will maintain compatible CLI syntax.  Still, this
was the best short term solution for the growing demand of containers
and wanting to have smaller and smaller installations.  The dnf
developers would have more time to come up with a long term solution.


.. _introducing_dnf5_ref-label:

Introducing dnf5
================

The developers decided that dnf was due for an overhaul given all of
the current requirements.  The decision was made that major version
5.x of dnf would be implemented in C++ but contain bindings to support
Python plugins as well as plugins written in other languages.  The
core functionality would be in C++ and would not require Python.
There would also be a shared library that other applications could
link with to gain dnf5 functionality.

A complete reimplementation of core system software is a big deal and
the team had to carefully plan both the introduction of and migration
to dnf5 in Fedora Linux.  This is part of the reason the name of the
software is dnf5 and not simply dnf.  During the migration process it
was possible to have both dnf and dnf5 installed concurrently.

These days the legacy dnf project is no longer actively developed,
though patches are still accepted from distributions that are using
it.  New releases are occassionally made of the legacy dnf project,
though that will likely stop in the future once no active OS is using
it.


.. _fedora_changes_ref-label:

Fedora Changes
==============

The sections below discuss the changes in each Fedora Linux release
that led to the eventual replacement of dnf with dnf5 in the core
system.


.. _fedora_linux_38_ref-label:

Fedora Linux 38
---------------

The first part of the migration was to get the new tools in the hands
of users.  The microdnf project was refactored to be based on the new
libdnf5 library meaning its backend would be the new dnf5 code base.
In Fedora Linux 38, this new microdnf release along with dnf5 and
libdnf5 were introduced to give users a new dnf stack to test and try
out.  The existing dnf stack remains the primary system dnf and
officially supported one.  Testers and early adopters of dnf5 at this
point are made aware of the current limitations and the incompatible
metadata between dnf and dnf5.

The change proposal: `MajorUpgradeOfMicrodnf
<https://fedoraproject.org/wiki/Changes/MajorUpgradeOfMicrodnf>`_


.. _fedora_linux_39_ref-label:

Fedora Linux 39
---------------

Discontinue modularity support in dnf5.  The Module Build Service was
planned for shut down after Fedora Linux 38 reached EOL, so this
release removed modularity support from the dnf5 stack.

The change proposal: `RetireModularity
<https://fedoraproject.org/wiki/Changes/RetireModularity>`_


.. _fedora_linux_40_ref-label:

Fedora Linux 40
---------------

The next major step was to modify the Fedora build system tools (mock,
koji, and copr) to use dnf5 instead of dnf.  The idea here is to use
the continually running build system as a burn in test for dnf5
robustness and determine if we are ready to switch to dnf5 on the core
system.

The change proposal: `BuildWithDNF5
<https://fedoraproject.org/wiki/Changes/BuildWithDNF5>`_

An additional change was made to the dnf code base.  Starting with
this release of Fedora, dnf will no longer download filelists by
default.  This is metadata in the package repository that describes
all of the files contained in each package.  It was determined that
this information is unnecessary in the majority of use cases, so the
default was switched to not download filelists.  This speeds up the
program for many users which improves overall user experience.

The change proposal: `DNFConditionalFilelists
<https://fedoraproject.org/wiki/Changes/DNFConditionalFilelists>`_


.. _fedora_linux_41_ref-label:

Fedora Linux 41
---------------

The final migration step is to move to dnf5 as the default on new
installs and system upgrades.  The existing dnf packages can remain
installed, but the default becomes dnf5 going forward.

The new dnf5 packages will change symlinks on the system.  Before
upgrading, you will have symlinks that look like this:

.. code-block:: shell

    $ tree /usr/bin/ -P dnf*
    /usr/bin/
    ├── dnf -> dnf-3
    ├── dnf-3
    └── dnf4 -> dnf-3

And after upgrading, they will look like this:

.. code-block:: shell

    $ tree /usr/bin/ -P dnf*
    /usr/bin/
    ├── dnf -> dnf5
    ├── dnf-3
    ├── dnf4 -> dnf-3
    └── dnf5

For new installs, the system will have symlinks that mirror the after
upgrading example.

While both versions of dnf can remain installed and both use the same
RPM database, the dnf state files differ between dnf4 and dnf5.  When
upgrading to dnf5, the system state is migrated from the existing dnf
to dnf5, but the transaction history is not moved.  You can continue
using dnf4 and dnf5, but transactions are not recorded concurrently in
each database.  Packages installed with dnf5 and viewed under dnf4
will show as user installed.  Likewise, packages installed with dnf4
and viewed under dnf5 will show as user installed.  The metadata
formats used for these databases in dnf are incompatible between dnf4
and dnf5.

dnf4 continues to exist as other projects begin their migrations to
dnf5 in Fedora.  Users are encouraged to use dnf5 exclusively for
daily operations.
