# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell upgrade-minimal


Background: Install glibc, flac, and CQRlib
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc flac CQRlib"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-0:2.28-9.fc29.x86_64                 |
        | install       | flac-0:1.3.2-8.fc29.x86_64                 |
        | install       | CQRlib-0:1.1.1-4.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | basesystem-0:11-6.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64          |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64   |


Scenario: Using dnf shell, make upgrade-minimal --bugfix
  Given I use repository "dnf-ci-fedora-updates"
   When I open dnf shell session
    And I execute in dnf shell "upgrade-minimal --bugfix glibc"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                    |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64                |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64         |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, make upgrade-minimal --security
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I open dnf shell session
    And I execute in dnf shell "upgrade-minimal --security CQRlib"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                    |
        | upgrade       | CQRlib-0:1.1.2-16.fc29.x86_64              |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario Outline: Using dnf shell, fail to upgrade-minimal <update> when no such upgrade is available
  Given I use repository "dnf-ci-fedora-updates"
    And I use repository "dnf-ci-fedora-updates-testing"
   When I open dnf shell session
    And I execute in dnf shell "upgrade-minimal <update> flac"
   Then Transaction is empty
    And stdout contains "No security updates needed"
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"

Examples:
  | update        |
  | --bugfix      |
  | --security    |


Scenario Outline: Using dnf shell, fail upgrade-minimal <update> for non-existent RPM
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I open dnf shell session
    And I execute in dnf shell "upgrade-minimal <update> non-existent"
   Then Transaction is empty
    And stdout contains "No match for argument"
    And stdout contains "No packages marked for upgrade"
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"

Examples:
  | update        |
  | --bugfix      |
  | --security    |
