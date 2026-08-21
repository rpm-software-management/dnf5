Feature: dnf download --debugsource command


Background:
  Given I use repository "dnf-ci-fedora-updates" as http
    And I set working directory to "{context.dnf.tempdir}"


Scenario: Download a debugsource for an RPM that doesn't exist
   When I execute dnf with args "download --debugsource does-not-exist"
   Then the exit code is 1
    And stderr contains "No package \"does-not-exist\" available"


Scenario: Download a debugsource for an existing RPM
   When I execute dnf with args "download --debugsource libzstd"
   Then the exit code is 0
    And stderr contains "zstd-debugsource-0:1.3.6-1.fc29.x86_64"
    And file sha256 checksums are following
        | Path                                                             | sha256                                                                                                       |
        | {context.dnf.tempdir}/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm |


Scenario: Download a debugsource with --destdir option
   When I execute dnf with args "download --debugsource libzstd --destdir={context.dnf.tempdir}/downloaddir"
   Then the exit code is 0
    And stderr contains "zstd-debugsource-0:1.3.6-1.fc29.x86_64"
    And file sha256 checksums are following
        | Path                                                                         | sha256                                                                                                       |
        | {context.dnf.tempdir}/downloaddir/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm |


Scenario: Download debugsource for all architectures
   When I execute dnf with args "download --debugsource lz4"
   Then the exit code is 0
    And stderr contains "lz4-debugsource-0:1.8.2-2.fc29.i686"
    And stderr contains "lz4-debugsource-0:1.8.2-2.fc29.x86_64"
    And file sha256 checksums are following
        | Path                                                           | sha256                                                                                                       |
        | {context.dnf.tempdir}/lz4-debugsource-1.8.2-2.fc29.i686.rpm    | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/i686/lz4-debugsource-1.8.2-2.fc29.i686.rpm      |
        | {context.dnf.tempdir}/lz4-debugsource-1.8.2-2.fc29.x86_64.rpm  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/lz4-debugsource-1.8.2-2.fc29.x86_64.rpm  |


Scenario: Download debugsource for specific architecture using --arch option
   When I execute dnf with args "download --debugsource --arch=x86_64 lz4"
   Then the exit code is 0
    And stderr contains "lz4-debugsource-0:1.8.2-2.fc29.x86_64"
    And stderr does not contain "lz4-debugsource-0:1.8.2-2.fc29.i686"
    And file sha256 checksums are following
        | Path                                                           | sha256                                                                                                       |
        | {context.dnf.tempdir}/lz4-debugsource-1.8.2-2.fc29.x86_64.rpm  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/lz4-debugsource-1.8.2-2.fc29.x86_64.rpm  |
        | {context.dnf.tempdir}/lz4-debugsource-1.8.2-2.fc29.i686.rpm    | -                                                                                                            |



Scenario: No error when debugsource package is not available
  Given I use repository "dnf-ci-fedora" as http
   When I execute dnf with args "download --debugsource basesystem"
   Then the exit code is 0
    # basesystem is a metapackage and doesn't have a debugsource package
    # The command succeeds but downloads nothing
    And stdout contains "Downloading Packages:"


Scenario: Download debugsource with --url option to print URLs instead of downloading
   When I execute dnf with args "download --debugsource --url libzstd"
   Then the exit code is 0
    And stdout contains "zstd-debugsource-1.3.6-1.fc29.x86_64.rpm"
    And file "{context.dnf.tempdir}/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm" does not exist


Scenario: Download debugsource from specific repository using --from-repo
   When I execute dnf with args "download --debugsource --from-repo=dnf-ci-fedora-updates libzstd"
   Then the exit code is 0
    And stderr contains "zstd-debugsource-0:1.3.6-1.fc29.x86_64"
    And file sha256 checksums are following
        | Path                                                             | sha256                                                                                                       |
        | {context.dnf.tempdir}/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm |


Scenario: Download multiple debugsource packages in one command
   When I execute dnf with args "download --debugsource libzstd lz4"
   Then the exit code is 0
    And stderr contains "zstd-debugsource-0:1.3.6-1.fc29.x86_64"
    And stderr contains "lz4-debugsource-0:1.8.2-2.fc29.i686"
    And stderr contains "lz4-debugsource-0:1.8.2-2.fc29.x86_64"
    And file sha256 checksums are following
        | Path                                                           | sha256                                                                                                       |
        | {context.dnf.tempdir}/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/zstd-debugsource-1.3.6-1.fc29.x86_64.rpm |
        | {context.dnf.tempdir}/lz4-debugsource-1.8.2-2.fc29.i686.rpm    | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/i686/lz4-debugsource-1.8.2-2.fc29.i686.rpm      |
        | {context.dnf.tempdir}/lz4-debugsource-1.8.2-2.fc29.x86_64.rpm  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/lz4-debugsource-1.8.2-2.fc29.x86_64.rpm  |
