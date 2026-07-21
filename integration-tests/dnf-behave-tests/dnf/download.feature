Feature: Download commands with options --resolve --alldeps


Scenario: Download a single rpm
  Given I use repository "dnf-ci-fedora"
    And I set working directory to "{context.dnf.tempdir}"
   When I execute dnf with args "download abcde"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.src.rpm" exists
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.noarch.rpm" exists


Scenario: Download with resolve option when no package is installed
  Given I use repository "dnf-ci-fedora"
    And I set working directory to "{context.dnf.tempdir}"
   When I execute dnf with args "download abcde --resolve"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.src.rpm" exists
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.noarch.rpm" exists
    And file "/{context.dnf.tempdir}/flac-1.3.2-8.fc29.x86_64.rpm" exists
    And file "/{context.dnf.tempdir}/wget-1.19.5-5.fc29.x86_64.rpm" exists


Scenario: Download with resolve option when a dependency is installed
  Given I use repository "dnf-ci-fedora"
    And I set working directory to "{context.dnf.tempdir}"
    And I successfully execute dnf with args "install wget"
   When I execute dnf with args "download abcde --resolve"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.src.rpm" exists
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.noarch.rpm" exists
    And file "/{context.dnf.tempdir}/flac-1.3.2-8.fc29.x86_64.rpm" exists
    And file "/{context.dnf.tempdir}/wget-1.19.5-5.fc29.x86_64.rpm" does not exist


@bz2066851
Scenario: Download with resolve respects install_weak_deps=false
  Given I use repository "dnf-ci-fedora"
    And I set working directory to "{context.dnf.tempdir}"
    And I configure dnf with
        | key                | value  |
        | install_weak_deps  | false  |
   When I execute dnf with args "download abcde --resolve"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.src.rpm" exists
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.noarch.rpm" exists
    And file "/{context.dnf.tempdir}/flac-1.3.2-8.fc29.x86_64.rpm" does not exist
    And file "/{context.dnf.tempdir}/lame-3.100-4.fc29.x86_64.rpm" does not exist
    And file "/{context.dnf.tempdir}/wget-1.19.5-5.fc29.x86_64.rpm" exists


Scenario: Download with resolve and alldeps options
  Given I use repository "dnf-ci-fedora"
    And I set working directory to "{context.dnf.tempdir}"
    And I successfully execute dnf with args "install wget"
   When I execute dnf with args "download abcde --resolve --alldeps"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.src.rpm" exists
    And file "/{context.dnf.tempdir}/abcde-2.9.2-1.fc29.noarch.rpm" exists
    And file "/{context.dnf.tempdir}/flac-1.3.2-8.fc29.x86_64.rpm" exists
    And file "/{context.dnf.tempdir}/wget-1.19.5-5.fc29.x86_64.rpm" exists

Scenario: Download with alldeps options
  Given I use repository "dnf-ci-fedora"
    And I set working directory to "{context.dnf.tempdir}"
   When I execute dnf with args "download abcde --alldeps"
   Then stderr contains "Option \"--alldeps\" should be used with \"--resolve\""
    And the exit code is 2

Scenario: Download when package is available from multiple repositories
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
    And I set working directory to "{context.dnf.tempdir}"
   When I execute dnf with args "download abcde --resolve"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/abcde-2.9.3-1.fc29.src.rpm" exists
    And file "/{context.dnf.tempdir}/abcde-2.9.3-1.fc29.noarch.rpm" exists
    And file "/{context.dnf.tempdir}/flac-1.3.3-3.fc29.x86_64.rpm" exists
    And file "/{context.dnf.tempdir}/wget-1.19.6-5.fc29.x86_64.rpm" exists


@bz1463723
Scenario: Download an existing RPM in file:// mode
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "download setup --destdir={context.dnf.tempdir}/downloaddir"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                              | sha256                                                                                     |
        | {context.dnf.tempdir}/downloaddir/setup-2.12.1-1.fc29.noarch.rpm  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm |


Scenario: Download an existing RPM in file:// mode with all dependencies into a --destdir
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "download basesystem --resolve --destdir={context.dnf.tempdir}/downloaddir"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                                  | sha256                                                                                        |
        | {context.dnf.tempdir}/downloaddir/setup-2.12.1-1.fc29.noarch.rpm      | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm    |
        | {context.dnf.tempdir}/downloaddir/basesystem-11-6.fc29.noarch.rpm     | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/basesystem-11-6.fc29.noarch.rpm   |
        | {context.dnf.tempdir}/downloaddir/filesystem-3.9-2.fc29.x86_64.rpm    | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/filesystem-3.9-2.fc29.x86_64.rpm  |


@bz1787908
Scenario: Download an existing RPM in file:// mode with all dependencies into a --destdir when it is in multiple repositories
  Given I use repository "dnf-ci-fedora"
# Add dnf-ci-fedora-updates repository with identical metadata of dnf-ci-fedora
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key             | value                                                 |
        | baseurl         | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora  |
   When I execute dnf with args "download basesystem --resolve --destdir={context.dnf.tempdir}/downloaddir"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                                  | sha256                                                                                        |
        | {context.dnf.tempdir}/downloaddir/setup-2.12.1-1.fc29.noarch.rpm      | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm    |
        | {context.dnf.tempdir}/downloaddir/basesystem-11-6.fc29.noarch.rpm     | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/basesystem-11-6.fc29.noarch.rpm   |
        | {context.dnf.tempdir}/downloaddir/filesystem-3.9-2.fc29.x86_64.rpm    | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/filesystem-3.9-2.fc29.x86_64.rpm  |


@bz1787908
Scenario: Download an existing RPM in two versions in file:// mode with all dependencies into a --destdir
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "download glibc-2.28-9.fc29 glibc-0:2.28-26.fc29.x86_64 --resolve --destdir={context.dnf.tempdir}/downloaddir"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                                            | sha256                                                                                                           |
        | {context.dnf.tempdir}/downloaddir/setup-2.12.1-1.fc29.noarch.rpm                | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm                       |
        | {context.dnf.tempdir}/downloaddir/basesystem-11-6.fc29.noarch.rpm               | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/basesystem-11-6.fc29.noarch.rpm                      |
        | {context.dnf.tempdir}/downloaddir/glibc-2.28-26.fc29.x86_64.rpm                 | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/glibc-2.28-26.fc29.x86_64.rpm                |
        | {context.dnf.tempdir}/downloaddir/glibc-2.28-9.fc29.x86_64.rpm                  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/glibc-2.28-9.fc29.x86_64.rpm                         |
        | {context.dnf.tempdir}/downloaddir/glibc-all-langpacks-2.28-26.fc29.x86_64.rpm   | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/glibc-all-langpacks-2.28-26.fc29.x86_64.rpm  |
        | {context.dnf.tempdir}/downloaddir/glibc-all-langpacks-2.28-9.fc29.x86_64.rpm    | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/glibc-all-langpacks-2.28-9.fc29.x86_64.rpm           |
        | {context.dnf.tempdir}/downloaddir/glibc-common-2.28-26.fc29.x86_64.rpm          | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/glibc-common-2.28-26.fc29.x86_64.rpm         |
        | {context.dnf.tempdir}/downloaddir/glibc-common-2.28-9.fc29.x86_64.rpm           | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/glibc-common-2.28-9.fc29.x86_64.rpm                  |
        | {context.dnf.tempdir}/downloaddir/filesystem-3.9-2.fc29.x86_64.rpm              | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/filesystem-3.9-2.fc29.x86_64.rpm                     |


@bz1800342
Scenario: Download RPM form repository of higher priority
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key             | value        |
        | priority        | 100          |
   When I execute dnf with args "download wget --destdir={context.dnf.tempdir}/downloaddir"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                             | sha256                                                                                     |
        | {context.dnf.tempdir}/downloaddir/wget-1.19.5-5.fc29.x86_64.rpm  | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/wget-1.19.5-5.fc29.x86_64.rpm  |


@bz2077864
Scenario: skip downloading of already downloaded rpm file
Given I use repository "simple-base" as http
  And I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
 When I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
 Then the exit code is 0
  And stderr contains "labirinto-0:1.0-1.fc29.src             100% \|   0.0   B/s \|   0.0   B \|  00m00s\n>>> Already downloaded"
  And stderr contains "labirinto-0:1.0-1.fc29.x86_64          100% \|   0.0   B/s \|   0.0   B \|  00m00s\n>>> Already downloaded"


@bz2077864
Scenario: re-download changed rpm file
Given I use repository "simple-base" as http
  And I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
  And I execute "echo blah > {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
 When I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
 # Verify we have redownloaded the labirinto rpm and it is installable
 Then I execute dnf with args "install {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
  And the exit code is 0


@bz2077864
Scenario: checksum cache is not used (rpm is re-downloaded) even when modified rpm has mtime and cache timestamp (sec format) in the same second
Given I use repository "simple-base" as http
  And I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
  And I execute "echo blah > {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
  And I execute "setfattr -n user.Librepo.checksum.mtime -v 1000000000 {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
  And I execute "touch --date @1000000000.000011111 {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
 When I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
 # Verify we have redownloaded the labirinto rpm and it is installable
 Then I execute dnf with args "install {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
  And the exit code is 0


@bz2077864
Scenario: checksum cache is not used (rpm is re-downloaded) even when modified rpm has mtime and cache timestamp (nsec format) in the same second
Given I use repository "simple-base" as http
  And I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
  And I execute "echo blah > {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
  And I execute "setfattr -n user.Librepo.checksum.mtime -v 1000000000000000000 {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
  And I execute "touch --date @1000000000.000011111 {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
 When I execute dnf with args "download labirinto --destdir={context.dnf.tempdir}/downloaddir"
 # Verify we have redownloaded the labirinto rpm and it is installable
 Then I execute dnf with args "install {context.dnf.tempdir}/downloaddir/labirinto-1.0-1.fc29.x86_64.rpm"
  And the exit code is 0
