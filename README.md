DNF5
====

DNF5 is a command-line package manager that automates the process of installing, upgrading, configuring, and removing computer programs in a consistent manner.
It supports RPM packages, modulemd modules, and comps groups & environments.

As part of the DNF5 stack, libdnf is the package management library.
It was originally written to support the [DNF](https://github.com/rpm-software-management/dnf/)
package manager and grew up into a versatile library.
Now you can use it for building custom tools that load repositories,
query packages, resolve dependencies and install packages.

It is powered with [libsolv](https://github.com/openSUSE/libsolv/), wrapping it with an easy to use programming interface.

* DNF5 supports working with the following artifacts:

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


:warning: **The current (main) branch is subject of a major rewrite. The API/ABI is currently unstable** :warning:


Documentation
=============

* For HTML documentation see https://dnf5.readthedocs.io/
* The header files are documented because documentation is mainly generated from them


Reporting issues
================

* [GitHub issues](https://github.com/rpm-software-management/dnf5/issues/new) [[backlog](https://github.com/rpm-software-management/dnf5/issues)]
* [Red Hat Bugzilla](https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=dnf5) [[backlog](https://bugzilla.redhat.com/buglist.cgi?bug_status=__open__&product=Fedora&component=dnf5)]


Contributing
============

* By contributing to this project you agree to the Developer Certificate of Origin (DCO).
  This document is a simple statement that you, as a contributor,
  have the legal right to submit the contribution. See the [DCO](DCO) file for details.
* All contributions to this project are licensed under [LGPLv2.1+](lgpl-2.1.txt) or [GPLv2+](gpl-2.0.txt).
  See the [License](#license) section for details.


Writing patches
---------------

* Please follow the [contributing guidelines](https://dnf5.readthedocs.io/en/latest/contributing/index.html)
* When a patch is ready, submit a pull request
* It is a good practice to write documentation and unit tests as part of the patches


Building
--------
To install build requirements, run::

    $ dnf builddep dnf5.spec [--define '_without_<option> 1 ...]

To build code, run::

    $ mkdir build
    $ cd build
    $ cmake .. [-DWITH_<OPTION>=<ON|OFF> ...]
    $ make -j4

To build rpms from git, run::

    $ export PREFIX=$(rpmspec dnf5.spec -q --srpm --qf '%{name}-%{version}'); git archive --format=tar.gz --prefix=$PREFIX/ HEAD > $PREFIX.tar.gz
    $ rpmbuild -ba --define "_sourcedir $(pwd)" dnf5.spec [--with=<option>|--without=<option> ...]


Testing
-------
To run the tests, follow the steps to build the code and then run::

    # from the 'build' directory
    $ CTEST_OUTPUT_ON_FAILURE=1 make test

As an alternative, tests can be executed in a verbose mode::

    # from the 'build' directory
    $ make test ARGS='-V'


Translating
-----------
TBD


License
=======

* The libraries is licensed under [LGPLv2.1+](lgpl-2.1.txt)
* The standalone programs that are part of this project are licensed under [GPLv2+](gpl-2.0.txt)
* See [COPYING](COPYING.md) for more details
