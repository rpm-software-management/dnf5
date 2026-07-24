# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell reinstall


Background: Install flac and filesystem
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
   When I execute dnf with args "install flac filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | filesystem-0:3.9-2.fc29.x86_64            |
        | install       | flac-0:1.3.2-8.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |


Scenario: Using dnf shell, reinstall an RPM
   When I open dnf shell session
    And I execute in dnf shell "reinstall flac"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                   |
        | reinstall     | flac-0:1.3.2-8.fc29.x86_64                |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, reinstall multiple RPMs
   When I open dnf shell session
    And I execute in dnf shell "reinstall flac filesystem"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                   |
        | reinstall     | filesystem-0:3.9-2.fc29.x86_64            |
        | reinstall     | flac-0:1.3.2-8.fc29.x86_64                |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to reinstall non-existent RPM
   When I open dnf shell session
    And I execute in dnf shell "reinstall NoSuchPackage"
   Then Transaction is empty
    And stdout contains "No match for argument"
    And stdout contains "No packages marked for reinstall"
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to reinstall an RPM when relevant repository is disabled
   When I open dnf shell session
    And I execute in dnf shell "repo disable dnf-ci-fedora"
    And I execute in dnf shell "repo enable dnf-ci-fedora-updates"
    And I execute in dnf shell "reinstall flac"
   Then Transaction is empty
    And stdout contains "Installed package.*flac-1.3.2-8.fc29.x86_64.*not available"
    And stdout contains "No packages marked for reinstall"
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"
