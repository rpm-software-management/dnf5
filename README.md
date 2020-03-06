libdnf
======

* Libdnf is a package management library.
* It's a backed of [DNF](https://github.com/rpm-software-management/dnf) package manager.
* Supported content types are:

  * RPM packages
  * repomd repositories
  * modulemd modules
  * comps - categories, environments and groups
  * updateinfo


:warning: **The current (dnf-5-devel) branch is subject of a major rewrite. The API/ABI is currently unstable** :warning:


Documentation
=============

* For HTML documentation see https://libdnf.readthedocs.io/
* The header files are documented because documentation is mainly generated from them


Reporting issues
================

* [Red Hat Bugzilla](https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=libdnf) is the preferred way of filing issues. [[backlog](https://bugzilla.redhat.com/buglist.cgi?bug_status=__open__&product=Fedora&component=libdnf)]
* [GitHub issues](https://github.com/rpm-software-management/libdnf/issues/new) are also accepted. [[backlog](https://github.com/rpm-software-management/libdnf/issues)]


Contributing
============

* By contributing to this project you agree to the Developer Certificate of Origin (DCO).
  This document is a simple statement that you, as a contributor,
  have the legal right to submit the contribution. See the [DCO](DCO) file for details.
* All contributions to this project are licensed under [LGPLv2.1+](lgpl-2.1.txt) or [GPLv2+](gpl-2.0.txt).
  See the [License](#license) section for details.


Writing patches
---------------

* Please follow the [coding style](CODING_STYLE.md)
* When a patch is ready, submit a pull request
* It is a good practice to write documentation and unit tests as part of the patches


Building
--------
To install build requirements, run::

    $ dnf builddep libdnf.spec --define '_with_sanitizers 1' [--define '_without_<option> 1 ...]

To build code, run::

    $ mkdir build
    $ cd build
    $ cmake .. -DWITH_SANITIZERS=ON [-DWITH_<OPTION>=<ON|OFF> ...]
    $ make -j4

To build rpms from git, run::

    $ export PREFIX=$(rpmspec libdnf.spec -q --srpm --qf '%{name}-%{version}'); git archive --format=tar.gz --prefix=$PREFIX/ HEAD > $PREFIX.tar.gz
    $ rpmbuild -ba --define "_sourcedir $(pwd)" libdnf.spec [--with=<option>|--without=<option> ...]


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
