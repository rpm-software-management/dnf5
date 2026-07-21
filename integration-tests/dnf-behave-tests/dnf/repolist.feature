Feature: Repo list (alias repolist)


Background: Using repositories dnf-ci-fedora and dnf-ci-thirdparty-updates
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty-updates"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
    And I use repository "dnf-ci-thirdparty" with configuration
        | key     | value |
        | enabled | 0     |


@dnf5daemon
Scenario: Repolist without arguments
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout contains "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout contains "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout does not contain "dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-thirdparty\s+dnf-ci-thirdparty"


# TODO(nsella) disable this until dnf5daemon-client suports `repo list` or some ad hoc test is created
# @dnf5daemon
Scenario: Repo list with "--enabled"
   When I execute dnf with args "repo list --enabled"
   Then the exit code is 0
    And stdout contains "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout contains "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout does not contain "dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-thirdparty\s+dnf-ci-thirdparty"


Scenario: Repo list with "--disabled"
   When I execute dnf with args "repo list --disabled"
   Then the exit code is 0
    And stdout contains "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates"
    And stdout contains "dnf-ci-thirdparty\s+dnf-ci-thirdparty"
    And stdout does not contain "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout does not contain "dnf-ci-thirdparty-updates"


Scenario: Repo list with "--all"
   When I execute dnf with args "repo list --all"
   Then the exit code is 0
    And stdout contains "dnf-ci-fedora\s+dnf-ci-fedora test repository\s+enabled"
    And stdout contains "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates test repository\s+disabled"
    And stdout contains "dnf-ci-thirdparty\s+dnf-ci-thirdparty test repository\s+disabled"
    And stdout contains "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates test repository\s+enabled"
