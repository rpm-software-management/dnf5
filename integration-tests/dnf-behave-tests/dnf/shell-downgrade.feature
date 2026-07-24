# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell downgrade


Background: Install glibc and flac
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-0:2.28-26.fc29.x86_64                |
        | install       | flac-0:1.3.3-3.fc29.x86_64                 |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | basesystem-0:11-6.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |


Scenario: Using dnf shell, downgrade an RPM
   When I open dnf shell session
    And I execute in dnf shell "downgrade glibc"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                    |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                 |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64          |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64   |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Downgrade multiple RPMs
   When I open dnf shell session
    And I execute in dnf shell "downgrade glibc flac"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                    |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                 |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64          |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64   |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64                 |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to downgrade an RPM of the lowest version
   When I open dnf shell session
    And I execute in dnf shell "downgrade setup"
   Then Transaction is empty
    # the following message is already updated for dnf5
    And stderr contains "The lowest available version of the \"setup.x86_64\" package is already installed, cannot downgrade it."
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to downgrade non-existent RPM
   When I open dnf shell session
    And I execute in dnf shell "downgrade non-existent"
   Then Transaction is empty
    And stdout contains "No package.*non-existent.*available"
    And stdout contains "No packages marked for downgrade"
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"
