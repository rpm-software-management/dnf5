Feature: Repositories with priorities


Background: Use repositories with priorities 1, 2, and 3
  Given I use repository "dnf-ci-priority-1" with configuration
        | key      | value |
        | priority | 1     |
    And I use repository "dnf-ci-priority-2" with configuration
        | key      | value |
        | priority | 2     |
    And I use repository "dnf-ci-priority-3" with configuration
        | key      | value |
        | priority | 3     |


Scenario: Install an RPM from the highest-priority repository and ensure that upgrade will not install update from the lower priority repository
   When I execute dnf with args "install flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.3-2.fc29.x86_64                |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is empty


Scenario: Install an RPM of specific version from lower-priority repository
   When I execute dnf with args "install flac-1.3.3-3.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.3-3.fc29.x86_64                |


Scenario: Install RPMs from different highest-priority repositories
   When I execute dnf with args "install flac *system"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.3-2.fc29.x86_64                |
        | install       | filesystem-0:3.9-2.fc29.x86_64            |
        | install       | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |


Scenario: Install an RPM and its dependencies from the proper highest-priority repositories
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |


Scenario: Upgrade an RPM from the highest-priority repository
   When I execute dnf with args "install flac-1.3.3-1.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.3-1.fc29.x86_64                |
   When I execute dnf with args "upgrade flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-2.fc29.x86_64                |


@bz1733582
Scenario: Do not upgrade installonly package from lower-priority repository
   When I execute dnf with args "install kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is empty


Scenario: Upgrade an RPM to specific version from lower-priority repository
   When I execute dnf with args "install flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.3-2.fc29.x86_64                |
   When I execute dnf with args "upgrade flac-1.3.3-3.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |


Scenario: Upgrade RPMs from different highest-priority repositories
   When I execute dnf with args "install setup glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install       | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
   When I drop repository "dnf-ci-priority-1"
   When I execute dnf with args "upgrade setup glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | upgrade       | setup-0:2.12.1-2.fc29.noarch               |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64                |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64         |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |


Scenario: Downgrade an RPM from the highest-priority repository
   When I execute dnf with args "install glibc-2.28-27.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-0:2.28-27.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | basesystem-0:11-6.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-27.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-27.fc29.x86_64  |
   When I execute dnf with args "downgrade glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                 |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64          |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64   |


Scenario: Downgrade an RPM to specific version from lower-priority repository
   When I execute dnf with args "install flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.3-2.fc29.x86_64                |
   When I execute dnf with args "downgrade flac-1.3.3-1.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | flac-0:1.3.3-1.fc29.x86_64                |


Scenario: Downgrade RPMs from different highest-priority repositories
   When I execute dnf with args "install setup-2.12.1-2.fc29 flac-1.3.3-3.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-2.fc29.noarch              |
        | install       | flac-0:1.3.3-3.fc29.x86_64                |
   When I execute dnf with args "downgrade setup flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | downgrade     | setup-0:2.12.1-1.fc29.noarch               |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64                 |
