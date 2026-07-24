# Tests for the dnf download command with --debuginfo options
# Implementation: https://github.com/rpm-software-management/dnf5/issues/948
Feature: dnf download --debuginfo command


Background:
  Given I set working directory to "{context.dnf.tempdir}"


Scenario: Download a debuginfo for an RPM that doesn't exist
  Given I use repository "dnf-ci-fedora" as http
   When I execute dnf with args "download --debuginfo does-not-exist"
   Then the exit code is 1
    And stderr contains "No package \"does-not-exist\" available"


Scenario: Download a debuginfo for an existing RPM
  Given I use repository "dnf-ci-fedora-updates" as http
   When I execute dnf with args "download --debuginfo libzstd"
   Then the exit code is 0
    And stderr contains "libzstd-debuginfo-0:1.3.6-1.fc29"
    And file sha256 checksums are following
        | Path                                                              | sha256                                                                                                        |
        | {context.dnf.tempdir}/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm |
        | {context.dnf.tempdir}/libzstd-1.3.6-1.fc29.x86_64.rpm             | -                                                                                                             |


Scenario: Download debuginfo for all architectures
  Given I use repository "dnf-ci-fedora-updates" as http
   When I execute dnf with args "download --debuginfo lz4"
   Then the exit code is 0
    And stderr contains "lz4-debuginfo-0:1.8.2-2.fc29"
    And file sha256 checksums are following
        | Path                                                         | sha256                                                                                                     |
        | {context.dnf.tempdir}/lz4-debuginfo-1.8.2-2.fc29.i686.rpm    | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/i686/lz4-debuginfo-1.8.2-2.fc29.i686.rpm      |
        | {context.dnf.tempdir}/lz4-debuginfo-1.8.2-2.fc29.x86_64.rpm  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/lz4-debuginfo-1.8.2-2.fc29.x86_64.rpm  |


@bz1637008
Scenario: Download source, debuginfo for an existing RPM
  Given I use repository "dnf-ci-fedora-updates" as http
   When I execute dnf with args "download --source --debuginfo libzstd"
   Then the exit code is 0
    And stderr contains "zstd-0:1.3.6-1.fc29"
    And stderr contains "libzstd-debuginfo-0:1.3.6-1.fc29"
    And file sha256 checksums are following
        | Path                                                              | sha256                                                                                                        |
        | {context.dnf.tempdir}/zstd-1.3.6-1.fc29.src.rpm                   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/src/zstd-1.3.6-1.fc29.src.rpm                    |
        | {context.dnf.tempdir}/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm |


Scenario: Download debuginfo with --destdir option
  Given I use repository "dnf-ci-fedora-updates" as http
   When I execute dnf with args "download --debuginfo libzstd --destdir={context.dnf.tempdir}/downloaddir"
   Then the exit code is 0
    And stderr contains "libzstd-debuginfo-0:1.3.6-1.fc29"
    And file sha256 checksums are following
        | Path                                                                      | sha256                                                                                                        |
        | {context.dnf.tempdir}/downloaddir/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm |


Scenario: Download debuginfo for multiple packages
  Given I use repository "dnf-ci-fedora-updates" as http
   When I execute dnf with args "download --debuginfo libzstd lz4"
   Then the exit code is 0
    And stderr contains "libzstd-debuginfo-0:1.3.6-1.fc29"
    And stderr contains "lz4-debuginfo-0:1.8.2-2.fc29"
    And file sha256 checksums are following
        | Path                                                              | sha256                                                                                                        |
        | {context.dnf.tempdir}/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/libzstd-debuginfo-1.3.6-1.fc29.x86_64.rpm |
        | {context.dnf.tempdir}/lz4-debuginfo-1.8.2-2.fc29.i686.rpm         | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/i686/lz4-debuginfo-1.8.2-2.fc29.i686.rpm         |
        | {context.dnf.tempdir}/lz4-debuginfo-1.8.2-2.fc29.x86_64.rpm       | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/lz4-debuginfo-1.8.2-2.fc29.x86_64.rpm     |
