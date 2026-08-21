Feature: for list --showduplicates option


Background: Enable repositories
Given I use repository "dnf-ci-fedora"
  And I use repository "dnf-ci-fedora-updates"
  And I use repository "dnf-ci-fedora-updates-testing"


@bz1671731
Scenario: Test for list with --showduplicates when the package is installed
 When I execute dnf with args "install flac-1.3.2"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                    |
      | install       | flac-0:1.3.2-8.fc29.x86_64 |
 When I execute dnf with args "list --showduplicates flac"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
 Then stdout matches line by line
      """
      Installed packages
      flac.x86_64\s+1.3.2-8.fc29\s+dnf-ci-fedora

      Available packages
      flac.src\s+1.3.2-8.fc29\s+dnf-ci-fedora
      flac.x86_64\s+1.3.2-8.fc29\s+dnf-ci-fedora
      flac.src\s+1.3.3-1.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-1.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.3.3-2.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-2.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.3.3-3.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-3.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      flac.x86_64\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      """


Scenario: Test for list without --showduplicates when the package is installed
 When I execute dnf with args "install flac-1.3.2"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                    |
      | install       | flac-0:1.3.2-8.fc29.x86_64 |
 When I execute dnf with args "list flac"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Installed packages
      flac.x86_64\s+1.3.2-8.fc29\s+dnf-ci-fedora

      Available packages
      flac.src\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      flac.x86_64\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      """


Scenario: Test for list with --showduplicates when the package is not installed
 When I execute dnf with args "update"
  And I execute dnf with args "list --showduplicates flac"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Available packages
      flac.src\s+1.3.2-8.fc29\s+dnf-ci-fedora
      flac.x86_64\s+1.3.2-8.fc29\s+dnf-ci-fedora
      flac.src\s+1.3.3-1.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-1.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.3.3-2.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-2.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.3.3-3.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-3.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      flac.x86_64\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      """


Scenario: Test for list without --showduplicates when the package is not installed
 When I execute dnf with args "update"
  And I execute dnf with args "list flac"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Available packages
      flac.src\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      flac.x86_64\s+1.4.0-1.fc29\s+dnf-ci-fedora-updates-testing
      """


@bz1655605
Scenario: Test for list --available --showduplicates when package is installed and repository is disabled
 When I drop repository "dnf-ci-fedora"
  And I drop repository "dnf-ci-fedora-updates-testing"
  And I execute dnf with args "install flac-1.3.3-1.fc29.x86_64"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                    |
      | install       | flac-0:1.3.3-1.fc29.x86_64 |
 When I execute dnf with args "list --available --showduplicates flac"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Available packages
      flac.src\s+1.3.3-1.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-1.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.3.3-2.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-2.fc29\s+dnf-ci-fedora-updates
      flac.src\s+1.3.3-3.fc29\s+dnf-ci-fedora-updates
      flac.x86_64\s+1.3.3-3.fc29\s+dnf-ci-fedora-updates
      """
 When I drop repository "dnf-ci-fedora-updates"
  And I execute dnf with args "list --available --showduplicates flac"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      No matching packages to list
      """
