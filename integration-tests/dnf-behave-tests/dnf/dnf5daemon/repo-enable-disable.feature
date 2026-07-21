@dnf5daemon
@not.with_mode=dnf5
Feature: Enable/disable repo functionality for dnf5daemon


Background:
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"


Scenario: Disable existing repositories
  Given I execute dnf with args "repo disable dnf-ci-fedora dnf-ci-fedora-updates"
   Then the exit code is 0
   When I execute dnf with args "repolist"
   Then stdout is empty


Scenario: Enable disabled repositories
  Given I execute dnf with args "repo disable dnf-ci-fedora dnf-ci-fedora-updates"
   Then the exit code is 0
   When I execute dnf with args "repo enable dnf-ci-fedora-updates"
   Then the exit code is 0
   When I execute dnf with args "repolist"
   Then stdout is
    """
    repo id               repo name
    dnf-ci-fedora-updates dnf-ci-fedora-updates test repository
    """


Scenario: Enable non-existing repositories
  Given I execute dnf with args "repo enable unknown-repo1 dnf-ci-fedora unknown-repo2"
   Then the exit code is 1
    And stderr is
    """
    [org.rpm.dnf.v0.rpm.Repo.NoMatchingIdError] No matching repositories found for following ids: unknown-repo1,unknown-repo2
    """
