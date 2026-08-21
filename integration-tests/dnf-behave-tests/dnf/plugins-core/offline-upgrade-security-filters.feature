Feature: Test the security filters for offline-upgrade commands


Background:
  Given I use repository "dnf-ci-fedora"


@bz1939975
Scenario: Test advisory filter with offline-upgrade
Given I execute dnf with args "install glibc flac"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action   | Package                                    |
      | upgrade  | flac-0:1.3.3-3.fc29.x86_64                 |
      | upgrade  | glibc-0:2.28-26.fc29.x86_64                |
      | upgrade  | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
      | upgrade  | glibc-common-0:2.28-26.fc29.x86_64         |
 When I execute dnf with args "offline-upgrade download --advisory FEDORA-2018-318f184000"
 Then the exit code is 0
  And DNF Transaction is following
      | Action   | Package                                    |
      | upgrade  | glibc-0:2.28-26.fc29.x86_64                |
      | upgrade  | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
      | upgrade  | glibc-common-0:2.28-26.fc29.x86_64         |


@xfail
# Unexpected reason change, reported as https://github.com/rpm-software-management/dnf5/issues/1831
@bz1939975
Scenario: Test bugfix filter with offline-upgrade
Given I use repository "dnf-ci-fedora-updates"
  And I execute dnf with args "install flac-1.3.2 kernel-4.18.16"
 Then the exit code is 0
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                                    |
      | install     | kernel-0:4.19.15-300.fc29.x86_64           |
      | upgrade     | flac-0:1.3.3-3.fc29.x86_64                 |
      | install-dep | kernel-core-0:4.19.15-300.fc29.x86_64      |
      | install-dep | kernel-modules-0:4.19.15-300.fc29.x86_64   |
 When I execute dnf with args "offline-upgrade download --bugfix"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                                    |
      | install     | kernel-0:4.19.15-300.fc29.x86_64           |
      | install-dep | kernel-core-0:4.19.15-300.fc29.x86_64      |
      | install-dep | kernel-modules-0:4.19.15-300.fc29.x86_64   |


@bz1939975
Scenario: Test bz filter with offline-upgrade
Given I execute dnf with args "install glibc flac"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action   | Package                                    |
      | upgrade  | flac-0:1.3.3-3.fc29.x86_64                 |
      | upgrade  | glibc-0:2.28-26.fc29.x86_64                |
      | upgrade  | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
      | upgrade  | glibc-common-0:2.28-26.fc29.x86_64         |
 When I execute dnf with args "offline-upgrade download --bz=222"
 Then the exit code is 0
  And DNF Transaction is following
      | Action   | Package                                    |
      | upgrade  | glibc-0:2.28-26.fc29.x86_64                |
      | upgrade  | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
      | upgrade  | glibc-common-0:2.28-26.fc29.x86_64         |


@bz1939975
Scenario: Test cve filter with offline-upgrade
Given I execute dnf with args "install glibc flac"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action   | Package                                    |
      | upgrade  | flac-0:1.3.3-3.fc29.x86_64                 |
      | upgrade  | glibc-0:2.28-26.fc29.x86_64                |
      | upgrade  | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
      | upgrade  | glibc-common-0:2.28-26.fc29.x86_64         |
 When I execute dnf with args "offline-upgrade download --cve=CVE-2999"
 Then the exit code is 0
  And DNF Transaction is following
      | Action   | Package                                    |
      | upgrade  | glibc-0:2.28-26.fc29.x86_64                |
      | upgrade  | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
      | upgrade  | glibc-common-0:2.28-26.fc29.x86_64         |


@xfail
# Unexpected reason change, reported as https://github.com/rpm-software-management/dnf5/issues/1831
@bz1939975
Scenario: Test enhancement filter with offline-upgrade
Given I use repository "dnf-ci-fedora-updates"
  And I use repository "enhancement-test"
  And I execute dnf with args "install flac-1.3.2 kernel-4.18.16"
 Then the exit code is 0
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                                    |
      | install     | kernel-0:4.19.15-300.fc29.x86_64           |
      | upgrade     | flac-0:1.3.9-1.fc29.x86_64                 |
      | install-dep | kernel-core-0:4.19.15-300.fc29.x86_64      |
      | install-dep | kernel-modules-0:4.19.15-300.fc29.x86_64   |
 When I execute dnf with args "offline-upgrade download --enhancement"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                                    |
      | upgrade     | flac-0:1.3.9-1.fc29.x86_64                 |


@bz1939975
Scenario: Test newpackage filter with offline-upgrade
Given I use repository "dnf-ci-fedora-updates"
  And I use repository "newpackage-test"
  And I execute dnf with args "install flac-1.3.2 somepackage-1.0"
 Then the exit code is 0
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                               |
      | upgrade     | flac-0:1.3.3-3.fc29.x86_64            |
      | upgrade     | somepackage-0:1.1-1.x86_64            |
 When I execute dnf with args "offline-upgrade download --newpackage"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                               |
      | upgrade     | somepackage-0:1.1-1.x86_64            |


@bz1939975
Scenario: Test security filter with offline-upgrade
Given I use repository "security-upgrade"
  And I execute dnf with args "install dracut-1-1 B-1-1"
 Then the exit code is 0
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                        |
      | upgrade     | B-0:2-2.x86_64                 |
      | upgrade     | dracut-0:2-2.x86_64            |
 When I execute dnf with args "offline-upgrade download --security"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                        |
      | upgrade     | B-0:2-2.x86_64                 |


@bz1939975
Scenario: Test security severity filter with offline-upgrade
Given I use repository "dnf-ci-security"
  And I execute dnf with args "install bugfix_B-1.0-1 advisory_B-1.0-3 security_A-1.0-1"
 Then the exit code is 0
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                        |
      | upgrade     | advisory_B-0:1.0-4.x86_64      |
      | upgrade     | bugfix_B-0:1.0-2.x86_64        |
      | upgrade     | security_A-0:1.0-4.x86_64      |
 When I execute dnf with args "offline-upgrade download --advisory-severities=Critical"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                        |
      | upgrade     | advisory_B-0:1.0-4.x86_64      |


@bz1939975
Scenario: Test security severity filter with offline-upgrade --minimal when higher version of a package is available
Given I use repository "dnf-ci-security"
  And I execute dnf with args "install bugfix_B-1.0-1 advisory_B-1.0-3 security_A-1.0-1"
 Then the exit code is 0
 When I execute dnf with args "offline-upgrade download"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                        |
      | upgrade     | advisory_B-0:1.0-4.x86_64      |
      | upgrade     | bugfix_B-0:1.0-2.x86_64        |
      | upgrade     | security_A-0:1.0-4.x86_64      |
 When I execute dnf with args "offline-upgrade download --minimal --advisory-severities=Moderate"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                        |
      | upgrade     | security_A-0:1.0-3.x86_64      |
