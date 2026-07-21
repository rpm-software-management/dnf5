Feature: Remove duplicate RPMs


@bz1674296
Scenario: Remove a duplicate RPM
  Given I use repository "dnf-ci-fedora-updates"
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-1.fc29.x86_64.rpm"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-3.fc29.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "remove --duplicates"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | flac-0:1.3.3-3.fc29.x86_64            |
        | obsoleted     | flac-0:1.3.3-1.fc29.x86_64            |


@bz1674296
@bz1647345
Scenario: Remove multiple duplicate RPMs
  Given I use repository "dnf-ci-fedora-updates"
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-1.fc29.x86_64.rpm"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-1.fc29.x86_64.rpm --force"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-1.fc29.x86_64.rpm --force"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-3.fc29.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "remove --duplicates"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | flac-0:1.3.3-3.fc29.x86_64            |
        | obsoleted     | flac-0:1.3.3-1.fc29.x86_64            |


Scenario: Installonly packages are excluded from duplicate removal
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
   When I execute dnf with args "remove --duplicates"
   Then the exit code is 0
    And Transaction is empty


Scenario: Remove duplicates filtered by package spec
  Given I use repository "dnf-ci-fedora-updates"
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-1.fc29.x86_64.rpm"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-3.fc29.x86_64.rpm"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/wget-1.19.5-5.fc29.x86_64.rpm"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/wget-1.19.6-5.fc29.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "remove --duplicates flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | flac-0:1.3.3-3.fc29.x86_64            |
        | obsoleted     | flac-0:1.3.3-1.fc29.x86_64            |


Scenario: The --duplicates option conflicts with --oldinstallonly
   When I execute dnf with args "remove --duplicates --oldinstallonly"
   Then the exit code is 2
    And stdout is empty
    And stderr is
      """
      "--oldinstallonly" not allowed together with named argument "--duplicates". Add "--help" for more information about the arguments.
      """


Scenario: The --limit option cannot be used with --duplicates
   When I execute dnf with args "remove --duplicates --limit=2"
   Then the exit code is 2
    And stdout is empty
    And stderr is
      """
      "--limit=2" not allowed together with named argument "--duplicates". Add "--help" for more information about the arguments.
      """


Scenario: Remove duplicates when newest version is not available in repos
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-1.fc29.x86_64.rpm"
   Then the exit code is 0
  Given I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-3.fc29.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "remove --duplicates"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | flac-0:1.3.3-1.fc29.x86_64            |


Scenario: No transaction when there are no duplicate packages
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install flac"
   Then the exit code is 0
   When I execute dnf with args "remove --duplicates"
   Then the exit code is 0
    And Transaction is empty
