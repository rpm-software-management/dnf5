DNF5
====

DNF5 is a command-line package manager that automates the process of installing, upgrading, configuring, and removing computer programs in a consistent manner.
It supports RPM packages, modulemd modules, and comps groups and environments.

As part of the DNF5 stack, libdnf is the package management library.
It was originally written to support the [DNF](https://github.com/rpm-software-management/dnf/)
package manager, but gradually grew up into a versatile library.
You can now use libdnf to build custom tools that load repositories,
query packages, resolve dependencies, and install packages.

DNF5 is also powered with the [libsolv](https://github.com/openSUSE/libsolv/) library which provides an easy to use programming interface.

By using DNF5, you can work with the following artifacts:

  * RPM repositories (repomd)
  * RPM packages
  * Comps groups
  * Comps environments
  * Advisories (updateinfo, errata)
  * Modules (modulemd)

DNF5 is written in C++ and it can interface with the following programming languages:

 * C++ - fully supported.
 * Python 3 - fully supported.
 * Perl 5 - best effort.
 * Ruby - best effort.

Note, however, that DNF5 cannot yet interface with the following programming languages:

 * Go - does not work, we are looking for contributors.
 * C - not implemented, not a priority for any of our existing API users.


Installing DNF5
---------------

To install the DNF5 package manager, use either of the following commands, depending on your Fedora release:

| Fedora release | Command |
| --------------------------- | ------- |
| 36 | `sudo dnf copr enable rpmsoftwaremanagement/dnf5-unstable && sudo dnf install dnf5`[^copr] |
| 37 | `sudo dnf copr enable rpmsoftwaremanagement/dnf5-unstable && sudo dnf install dnf5`[^copr] |
| 38                 | `sudo dnf install dnf5` |
[^copr]: https://copr.fedorainfracloud.org/coprs/rpmsoftwaremanagement/dnf5-unstable/

Documentation
=============

* [DNF5 HTML Documentation](https://dnf5.readthedocs.io/)
* The header files are documented because documentation is mainly generated from these files.


Contributing to the DNF5 project
================================

For details about how to contribute to the DNF5 project, see [CONTRIBUTING.md](https://github.com/rpm-software-management/dnf5/blob/main/CONTRIBUTING.md).

* By contributing to the DNF5 project you agree with the Developer Certificate of Origin (DCO).
  DCO contains a simple statement that you, as a contributor,
  have the legal right to submit a contribution. For more details, see the [DCO](DCO) file.
* All contributions to this project are licensed under [LGPLv2.1+](lgpl-2.1.txt) or [GPLv2+](gpl-2.0.txt).
  For more details, see the [License](#license) section.


Reporting issues
================

To report an issue, use either of the following methods:

* Create a [GitHub issue](https://github.com/rpm-software-management/dnf5/issues/new) [[backlog](https://github.com/rpm-software-management/dnf5/issues)].
* File a bug by using the [Red Hat Bugzilla](https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=dnf5) tool.
  [[backlog](https://bugzilla.redhat.com/buglist.cgi?bug_status=__open__&product=Fedora&component=dnf5)]


Developing DNF5 by building and testing the code
------------------------------------------------

For details about building and testing DNF5, see the [Development environment setup](https://github.com/rpm-software-management/dnf5/blob/main#development-environment-setup) section.

Translating
-----------
TBD


License
=======

* The DNF5 libraries are licensed under [LGPLv2.1+](lgpl-2.1.txt).
* The standalone programs that are part of the DNF5 project are licensed under [GPLv2+](gpl-2.0.txt).

For more details about licenses, see [COPYING](COPYING.md).
