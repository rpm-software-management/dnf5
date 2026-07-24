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

Join us on IRC at `#dnf` on [Libera.Chat](https://libera.chat). Questions should be asked there, issues discussed. Remember: `#dnf` is not a support channel, and prior research is expected from the questioner.


Installing DNF5
---------------

DNF5 is available since Fedora 38. To install the DNF5 package manager, use either of the following commands, depending on your Fedora release:

| Fedora release | Command |
| --------------------------- | ------- |
| 37 | `sudo dnf copr enable rpmsoftwaremanagement/dnf-nightly && sudo dnf install dnf5`[^dnf-nightly] |
| 38 or newer                 | `sudo dnf install dnf5` |
[^dnf-nightly]: https://copr.fedorainfracloud.org/coprs/rpmsoftwaremanagement/dnf-nightly

Note: `dnf-nightly` provides nightly builds for the entire DNF stack. Once you enable this repository you will start receiving updates for DNF, libdnf, and for the other packages of the stack.


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

To report an issue (except of issues in the translations), use either of the following methods:

* Create a [GitHub issue](https://github.com/rpm-software-management/dnf5/issues/new) [[backlog](https://github.com/rpm-software-management/dnf5/issues)].
* File a bug by using the [Red Hat Bugzilla](https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=dnf5) tool.
  [[backlog](https://bugzilla.redhat.com/buglist.cgi?bug_status=__open__&product=Fedora&component=dnf5)]

To report an issue in the translated messages:

* Correct the message yourself at [Fedora Weblate](https://translate.fedoraproject.org/projects/dnf5/).
* Mark the message as _Needs editting_ at the same place.
* Contact a translator of the affected translation catalog directly.


Developing DNF5 by building and testing the code
------------------------------------------------

For details about building and testing DNF5, see the [Development environment setup](CONTRIBUTING.md#setting-up-a-development-environment) section.


Running Tests
-------------

### Unit Tests

Build the project and run the C++ unit tests:
```
make test-unit
```

To run a specific subset of unit tests:
```
make test-unit CTEST_ARGS="-R libdnf5"
```

### Integration Tests

The integration test suite is based on [behave](https://behave.readthedocs.io/)
(BDD framework using Gherkin `.feature` files) and runs inside containers for
sandboxing.

The test suite is located in [`integration-tests/dnf-behave-tests/`](integration-tests/dnf-behave-tests/).
For documentation of the test suite structure, steps and conventions, see
[`integration-tests/dnf-behave-tests/README.md`](integration-tests/dnf-behave-tests/README.md).

To build RPMs from your local source, build the test container, and run the
dnf5 integration tests:
```
make test-integration ARGS="--tags dnf5 --command dnf5"
```

To run a single feature file:
```
make test-integration ARGS="--command dnf5 config.feature"
```

To run dnf5daemon integration tests:
```
make test-integration ARGS="--tags dnf5daemon --command dnf5daemon-client"
```

To skip destructive tests:
```
make test-integration ARGS="--no-destructive --command dnf5"
```

To reserve a shell session inside the container on test failure (useful for debugging):
```
make test-integration ARGS="-R --command dnf5"
```

If the container is already built, you can skip the RPM build and container
build steps and just re-run the tests:
```
make test-integration-run ARGS="--command dnf5"
```

For development, you can also run the `container-test` script directly:
```
./integration-tests/container-test --help
./integration-tests/container-test run --help
```

#### Building the Container Image

The container image is based on Fedora and installs the latest DNF stack from
the dnf-nightly Copr repository. Any RPMs found in `integration-tests/rpms/`
are installed on top, which is how local changes are tested.

To build the image on a different base:
```
./integration-tests/container-test build --base quay.io/centos/centos:stream10
```

To build without the nightly Copr (distro packages only):
```
./integration-tests/container-test build --type distro
```

Additional CA certificates can be placed in `integration-tests/ca-trust/` and
extra repo files in `integration-tests/repos.d/`.

#### Running Tests Directly (Without Containers)

First build the test data:
```
cd integration-tests/dnf-behave-tests
fixtures/specs/build.sh
```

Then run behave directly (requires root for most tests):
```
sudo behave -Ddnf_command=dnf5 dnf
sudo behave -Ddnf_command=dnf5 dnf/config.feature
sudo behave -Ddnf_command=dnf5 -n "Test scenario name" dnf/config.feature
sudo behave -Ddnf_command=dnf5 -Ddestructive=yes dnf/cache.feature
```

Translating
-----------

Translating DNF5 from English to other languages happens at
[Fedora Weblate](https://translate.fedoraproject.org/projects/dnf5/)
translation web site.

A [GitHub workflow](.github/workflows/weblate-sync-pot.yml) extracts new English
messages from this repository every day and commits them to
[dnf5-l10n](https://github.com/rpm-software-management/dnf5-l10n) repository.
The translation web site then presents them to its users who translate
them there and the web site saves the finished translations back to the
dnf5-l10n repository.
Finally, the translation catalogs from that repository are
[copied](.github/actions/weblate-pull-translations/action.yml) into this repository
just before [every DNF5 release](.github/workflows/prepare-release.yml).


License
=======

* The DNF5 libraries are licensed under [LGPLv2.1+](lgpl-2.1.txt).
* The standalone programs that are part of the DNF5 project are licensed under [GPLv2+](gpl-2.0.txt).

For more details about licenses, see [COPYING](COPYING.md).
