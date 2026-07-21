Feature: dnf download --srpm command


Background:
  Given I use repository "dnf-ci-fedora" as http
    And I set working directory to "{context.dnf.tempdir}"


@xfail
# see https://github.com/rpm-software-management/dnf5/issues/1151
Scenario: Download a source for an RPM that doesn't exist
   When I execute dnf with args "download --srpm does-not-exist"
   Then the exit code is 1
    And stderr contains "No package does-not-exist available"


Scenario: Download a source for an existing RPM
   When I execute dnf with args "download --srpm setup"
   Then the exit code is 0
    And stderr contains "setup-0:2.12.1-1.fc29.src"
    And file sha256 checksums are following
        | Path                                              | sha256                                                                                |
        | {context.dnf.tempdir}/setup-2.12.1-1.fc29.src.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/src/setup-2.12.1-1.fc29.src.rpm  |


Scenario: Download a source for an existing RPM with a different name
   When I execute dnf with args "download --srpm nscd"
   Then the exit code is 0
    And stderr contains "glibc-0:2.28-9.fc29.src"
    And file sha256 checksums are following
        | Path                                              | sha256                                                                                |
        | {context.dnf.tempdir}/glibc-2.28-9.fc29.src.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/src/glibc-2.28-9.fc29.src.rpm    |


@xfail
# see https://github.com/rpm-software-management/dnf5/issues/580
Scenario: Download an existing --srpm RPM with --verbose option
   When I execute dnf with args "download --srpm setup --verbose"
   Then the exit code is 0
    And stderr contains "setup-2.12.1-1.fc29.src.rpm"
    And file sha256 checksums are following
        | Path                                              | sha256                                                                |
        | {context.dnf.tempdir}/setup-2.12.1-1.fc29.src.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/src/setup-2.12.1-1.fc29.src.rpm  |


@bz1649627
Scenario: Download a specified source rpm
   When I execute dnf with args "download --destdir={context.dnf.tempdir}/downloaddir --srpm setup-2.12.1-1.fc29.src"
   Then the exit code is 0
    And stderr contains "setup-0:2.12.1-1.fc29.src"
    And stderr does not contain "setup-0:2.12.1-1.fc29.noarch"
    And file sha256 checksums are following
        | Path                                                              | sha256                                                                                |
        | {context.dnf.tempdir}/downloaddir/setup-2.12.1-1.fc29.src.rpm     | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/src/setup-2.12.1-1.fc29.src.rpm  |
        | {context.dnf.tempdir}/downloaddir/setup-2.12.1-1.fc29.noarch.rpm  | -                                                                                     |


@bz1649627
Scenario Outline: Download a source RPM when there are more versions available
  Given I use repository "dnf-ci-fedora-updates-testing" as http
   When I execute dnf with args "download --srpm <pkgspec>"
   Then the exit code is 0
    And stderr contains "<srpm nevra>"
    And file sha256 checksums are following
        | Path                          | sha256                                                                           |
        | {context.dnf.tempdir}/<srpm>  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates-testing/src/<srpm> |

Examples:
  | pkgspec                     | srpm                           | srpm nevra                     |
  | wget                        | wget-1.19.5-5.fc29.src.rpm     | wget-1:1.19.5-5.fc29.src       |
  | wget-1.19.4-1.fc29          | wget-1.19.4-1.fc29.src.rpm     | wget-1:1.19.4-1.fc29.src       |
  | wget-1.19.5-5.fc29          | wget-1.19.5-5.fc29.src.rpm     | wget-1:1.19.5-5.fc29.src       |
  | wget-1.19.4-1.fc29.src      | wget-1.19.4-1.fc29.src.rpm     | wget-1:1.19.4-1.fc29.src       |
  | wget-1.19.5-5.fc29.src      | wget-1.19.5-5.fc29.src.rpm     | wget-1:1.19.5-5.fc29.src       |
  | wget-1.19.4-1.fc29.x86_64   | wget-1.19.4-1.fc29.src.rpm     | wget-1:1.19.4-1.fc29.src       |
  | wget-1.19.5-5.fc29.x86_64   | wget-1.19.5-5.fc29.src.rpm     | wget-1:1.19.5-5.fc29.src       |


Scenario Outline: Download a source RPM when there are more epochs available
  Given I use repository "dnf-ci-fedora-updates-testing" as http
   When I execute dnf with args "download --srpm <pkgspec>"
   Then the exit code is 0
    And stderr contains "<srpm nevra>"
    And file sha256 checksums are following
        | Path                          | sha256                                                    |
        | {context.dnf.tempdir}/<srpm>  | file://{context.dnf.fixturesdir}/repos/<repo>/src/<srpm>  |

Examples:
  | pkgspec                     | repo                           | srpm                           | srpm nevra                   |
  | wget-0:1.19.5-5.fc29        | dnf-ci-fedora                  | wget-1.19.5-5.fc29.src.rpm     | wget-0:1.19.5-5.fc29.src     |
  | wget-1:1.19.4-1.fc29        | dnf-ci-fedora-updates-testing  | wget-1.19.4-1.fc29.src.rpm     | wget-1:1.19.4-1.fc29.src     |
  | wget-1:1.19.5-5.fc29        | dnf-ci-fedora-updates-testing  | wget-1.19.5-5.fc29.src.rpm     | wget-1:1.19.5-5.fc29.src     |
  | wget-0:1.19.5-5.fc29.src    | dnf-ci-fedora                  | wget-1.19.5-5.fc29.src.rpm     | wget-0:1.19.5-5.fc29.src     |
  | wget-1:1.19.4-1.fc29.src    | dnf-ci-fedora-updates-testing  | wget-1.19.4-1.fc29.src.rpm     | wget-1:1.19.4-1.fc29.src     |
  | wget-1:1.19.5-5.fc29.src    | dnf-ci-fedora-updates-testing  | wget-1.19.5-5.fc29.src.rpm     | wget-1:1.19.5-5.fc29.src     |
  | wget-0:1.19.5-5.fc29.x86_64 | dnf-ci-fedora                  | wget-1.19.5-5.fc29.src.rpm     | wget-0:1.19.5-5.fc29.src     |
  | wget-1:1.19.4-1.fc29.x86_64 | dnf-ci-fedora-updates-testing  | wget-1.19.4-1.fc29.src.rpm     | wget-1:1.19.4-1.fc29.src     |
  | wget-1:1.19.5-5.fc29.x86_64 | dnf-ci-fedora-updates-testing  | wget-1.19.5-5.fc29.src.rpm     | wget-1:1.19.5-5.fc29.src     |


# TODO: --srpm --resolve doesn't work correctly; see see bug 1571251
