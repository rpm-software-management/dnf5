Feature: Repo list (alias repolist) when all repositories are disabled

Background:
  Given I use repository "dnf-ci-fedora" with configuration
        |key      | value |
        | enabled | 0     |


Scenario: Repolist without arguments
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout is empty


Scenario: Repo list with "--enabled"
   When I execute dnf with args "repo list --enabled"
   Then the exit code is 0
    And stdout is empty


Scenario: Repo list with "--disabled"
   When I execute dnf with args "repo list --disabled"
   Then the exit code is 0
    And stdout is
      """
      repo id       repo name
      dnf-ci-fedora dnf-ci-fedora test repository
      """


Scenario: Repo list with "--all"
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "repo list --all"
   Then the exit code is 0
    And stdout is
      """
      repo id               repo name                               status
      dnf-ci-fedora         dnf-ci-fedora test repository         disabled
      dnf-ci-fedora-updates dnf-ci-fedora-updates test repository  enabled
      """


Scenario: Repo list with "--disabled --enabled"
   When I execute dnf with args "repo list --disabled --enabled"
   Then the exit code is 2
    And stdout is empty
    And stderr is
      """
      "--enabled" not allowed together with named argument "--disabled". Add "--help" for more information about the arguments.
      """
