# Contributing Guide

* [Ways to contribute](#ways-to-contribute)
* [Finding an issue](#finding-an-issue)
* [Asking for help](#asking-for-help)
* [Submitting a contribution](#submitting-a-contribution)
* [Setting up a development environment](#setting-up-a-development-environment)
* [Pull request checklist](#pull-request-checklist)

Welcome to the DNF5 project! We are glad that you want to contribute!

We welcome your feedback on different areas of our project, including the following:

* Problems found during a new developer environment setup.
* Gaps in our documentation.
* Bugs in automation scripts.

If you are experiencing problems with the project, let us know by opening an
[issue](https://github.com/rpm-software-management/dnf5/issues/new).
If you want to file a bug on Fedora, you can open the bug in the
[Bugzilla](https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=dnf5) tool.

## Ways to contribute

We welcome different types of contributions, including the following:

* [New features](https://github.com/rpm-software-management/dnf5/labels/RFE)
* [Bug fixes](https://bugzilla.redhat.com/buglist.cgi?bug_status=__open__&product=Fedora&component=dnf5)
* [Documentation](https://dnf5.readthedocs.io)
* [Issue triage](https://github.com/rpm-software-management/dnf5/issues)
* [Copr builds](https://copr.fedorainfracloud.org/coprs/rpmsoftwaremanagement/), [CI/CD](https://github.com/rpm-software-management/dnf5/actions)
* [Communications](#asking-for-help) and [GH discussions](https://github.com/rpm-software-management/dnf5/discussions)

## Finding an issue

If you are a new contributor, you can search for issues with the
[_good first issue_](https://github.com/rpm-software-management/dnf5/labels/good%20first%20issue) label.
These issues are suitable for contributors who are not core maintainers and want to get familiar with the codebase.

Note that sometimes there can be no issues with the _good first issue_ label. Do not worry! There is likely still
something for you to work on. If you want to contribute but you do not know where to start or cannot find a suitable
issue, ask for help by reaching out to us on one of our [channels](#asking-for-help).

If there is an issue that you would like to work on, add a comment saying
that you want to work on this issue, for example, "I want to work on this", and assign the issue to yourself.

## Asking for help

If you need any help when contributing to the project, reach out to us either of the following ways:

* Leave a comment in the original github issue.
* Send a message to our mailing list: [sst-cs-software-management@redhat.com](mailto:sst-cs-software-management@redhat.com)
* Ask us on our IRC channel: [#dnf](irc:/chat.libera.org/#dnf)

## Submitting a contribution

Once you have a contribution ready, complete the following steps to submit the contribution:

1. Submit a pull request (PR).
    * If the PR is not yet ready to be reviewed by the maintainers, create a
      "Draft Pull Request". Then, when it's ready, mark it as "Ready for
      review".
2. Review the PR.
   1. Discuss the PR with one of the maintainers. Note that the maintainers might request some changes.
   2. Review the PR again.
3. Merge the PR.

## Setting up a development environment

To setup a development environment, complete the following steps:

1. Install build requirements:

   ```
   dnf builddep dnf5.spec #[--define '_without_<option> 1 ...]
   ```

2. Build DNF5:

   ```
   mkdir build
   cd build
   cmake ..
   # or cmake .. [-DWITH_<OPTION>=<ON|OFF> ...]
   make
   ```

3. Run the unit-tests. To run the tests, follow the steps to build the code and then run this code:

   ```
   # from the 'build' directory
   CTEST_OUTPUT_ON_FAILURE=1 make test
   ```

   Alternatively, you can execute tests in a verbose mode:

   ```
   # from the 'build' directory
   make test ARGS='-V'
   ```

4. Install DNF5. To install the built package, we recommend to build an rpm package.

   You can build the rpm package either of the following ways:

   - Build rpms by using tito:

     ```
     tito build --rpm --test
     ```

   - Build rpms from git:

     ```
     export PREFIX=$(rpmspec dnf5.spec -q --srpm --qf '%{name}-%{version}'); git archive --format=tar.gz --prefix=$PREFIX/ HEAD > $PREFIX.tar.gz
     rpmbuild -ba --define "_sourcedir $(pwd)" dnf5.spec #[--with=<option>|--without=<option> ...]
     ```

## License Header

New source files should be licensed under the **GNU Lesser General Public License v2.1 or later**.
Specify the license using `SPDX-License-Identifier`.

New C++ and C source files should start with the following header:
```
// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later
```

The same rule applies to other source files, with the header adjusted according to the rules of the specific language. For example, Perl, Python, and Ruby source files should start with the header:
```
# Copyright Contributors to the DNF5 project
# SPDX-License-Identifier: LGPL-2.1-or-later
```

## Pull request checklist

When you submit your pull request or push new commits to it, our automated
systems will run some checks on your new code. We require that your pull request
passes these checks. However, note that there are more criteria that your pull request must pass before we can
accept and merge it. We recommend that you ensure the following locally
before you submit your code:

* Code must build.

  Ensure that your code builds [locally](#setting-up-a-development-environment) unless you need some help from us or you need us
  to review work in progress changes.

* Code must be tested.

  If your pull request includes new or modified functionality within the library, we kindly request that you provide
  matching unit tests in the project's test directory to cover these changes. However, if the changes only affect
  the command-line interface, you can provide related CI tests in the
  [ci-dnf-stack](https://github.com/rpm-software-management/ci-dnf-stack) component.
  If you need our assistance, ask the maintainers for help.

* Code must pass sanity checks.

  Test the sanity of the codebase by performing [pre-commit](https://pre-commit.com/) checks.
  In the `dnf5` directory, run the following commands:

  ```bash
  pre-commit install
  git add _<your_changes>_
  git commit -m "_<your_commit>_"
  ...
  Check the results.

  ```

  All checks will run as part of a PR action on GitHub. Therefore, make sure not to skip the checks if you do not
  want to amend your contribution.

  As part of pre-commit checks, we perform checks such as trailing whitespaces, end of file fixes, clang-format,
  and rpmlint checks. For more information,
  see [.pre-commit-config.yml](https://github.com/rpm-software-management/dnf5/blob/main/.pre-commit-config.yaml).
