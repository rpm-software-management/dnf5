# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell repo


Scenario: Using dnf shell, enable repositories
  Given I use repository "dnf-ci-fedora" with configuration
        | key     | value |
        | enabled | 0     |
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
    And I use repository "dnf-ci-fedora-updates-testing" with configuration
        | key     | value |
        | enabled | 0     |
   When I open dnf shell session
    And I execute in dnf shell "repo enable dnf-ci-fedora*"
    And I execute in dnf shell "repolist"
   Then stdout contains "dnf-ci-fedora"
    And stdout contains "dnf-ci-fedora-updates"
    And stdout contains "dnf-ci-fedora-updates-testing"
    And stdout does not contain "dnf-ci-thirdparty"
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, disable repositories
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I open dnf shell session
    And I execute in dnf shell "repo disable dnf-ci-fedora-updates"
    And I execute in dnf shell "repolist"
   Then stdout contains "dnf-ci-fedora"
    And stdout does not contain "dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-thirdparty"
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, disable and enable repositories
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
    And I use repository "dnf-ci-fedora-updates-testing"
   When I open dnf shell session
    And I execute in dnf shell "repo disable dnf-ci-fedora-updates*"
    And I execute in dnf shell "repo enable dnf-ci-fedora-updates"
    And I execute in dnf shell "repolist"
   Then stdout contains "dnf-ci-fedora"
    And stdout contains "dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-fedora-updates-testing"
    And stdout does not contain "dnf-ci-thirdparty"
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to enable non-existent repository
   When I open dnf shell session
    And I execute in dnf shell "repo enable NoSuchRepo"
   Then stdout contains "Error: Unknown repo: '.*NoSuchRepo.*'"
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to disable non-existent repository
   When I open dnf shell session
    And I execute in dnf shell "repo disable NoSuchRepo"
   Then stdout contains "Error: Unknown repo: '.*NoSuchRepo.*'"
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"
