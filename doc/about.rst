=====
About
=====

DNF5 is a command-line package manager that automates the process of installing, upgrading, configuring, and removing computer programs in a consistent manner.
It supports RPM packages, modulemd modules, and comps groups & environments.

As part of the DNF5 stack, libdnf5 is the package management library.
It was originally written to support the `DNF <https://github.com/rpm-software-management/dnf/>`_,
package manager and grew up into a versatile library.
Now we can use it for building custom tools that load repositories,
query packages, resolve dependencies and install packages.

It is powered with `libsolv <https://github.com/openSUSE/libsolv/>`_,
wrapping it with an easy to use programming interface.

DNF5 supports working with the following artifacts:

 * RPM repositories (repomd)
 * RPM packages
 * Comps groups
 * Comps environments
 * Advisories (updateinfo, errata)
 * Modules (modulemd)

DNF5 is written in C++ and it can interface with several programming languages:

 * C++ - fully supported
 * Python 3 - fully supported
 * Perl 5 - best effort
 * Ruby - best effort
 * Go - doesn't work, looking for contributors
 * C - not implemented, doesn't seem to be a priority for any of our existing API users


License
=======

* The libraries is licensed under `LGPLv2.1+ <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt>`_
* The standalone programs that are part of this project are licensed under `GPLv2+ <https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>`_
* See COPYING.md for more details


Authors
=======

DNF5 has been developed mainly by the Red Hat's Software Management team.
Many features, bug fixes and bug reports came also from the community,
see AUTHORS.md and git history for the complete list.
