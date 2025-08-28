.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _debuginfo_install_command_ref-label:

##########################
 Debuginfo-install Command
##########################

Synopsis
========

``dnf5 debuginfo-install [options] <package-spec-NPFB>...``


Description
===========

Install the associated debuginfo packages for a given package specification.
The command temporary enables corresponding debug repository for each enabled
repository using following algorithm. When enabled repository ID has suffix `-rpm`
then it enables <ID>-debug-rpms. When enabled repository does not have suffix `-rpm`
it enables repository using pattern <ID>-debuginfo.

When regular upgrade of debuginfo packages is expected, then it requires enabling
of debug repository permanently using `config-manager` command.

Options
=======

``--allowerasing``
    | Allow removing of installed packages to resolve any potential dependency problems.

``--skip-broken``
    | Resolve any dependency problems by removing packages that are causing problems from the transaction.

``--skip-unavailable``
    | Allow skipping packages that are not available in repositories. All available packages will be installed.

.. include:: ../_shared/options/transaction.rst

Examples
========

``dnf debuginfo-install foobar``
    Install the debuginfo packages for the foobar package.

``dnf upgrade --enablerepo=*-debuginfo <package-name>-debuginfo``
    Upgrade debuginfo package of a <package-name>.

``dnf upgrade --enablerepo=*-debuginfo "*-debuginfo"``
    Upgrade all debuginfo packages.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
