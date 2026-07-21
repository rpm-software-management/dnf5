# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell upgrade


Background: Install flac
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
   When I execute dnf with args "install flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.2-8.fc29.x86_64                |


Scenario: Using dnf shell, upgrade an RPM
   When I open dnf shell session
    And I execute in dnf shell "repo enable dnf-ci-fedora-updates"
    And I execute in dnf shell "upgrade flac"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, upgrade an RPM with the "update" alias
   When I open dnf shell session
    And I execute in dnf shell "repo enable dnf-ci-fedora-updates"
    And I execute in dnf shell "update flac"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"
