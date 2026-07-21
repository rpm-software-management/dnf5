Feature: transaction: upgrades and downgrades

Background: Install RPMs
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install       | flac-0:1.3.2-8.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |


Scenario: Construct query and upgrade flac package
  Given I use repository "dnf-ci-fedora-updates"
   When I execute python libdnf5 api script with setup
        """
        goal = libdnf5.base.Goal(base)
        goal.add_rpm_upgrade("flac")
        execute_transaction(goal, "upgrade a package without dependencies")
        """
   Then the exit code is 0
    And stdout is
        """
        flac-1.3.3-3.fc29.x86_64 : Upgrade
        flac-1.3.2-8.fc29.x86_64 : Replaced
        """
    And RPMDB Transaction is following
        | Action        | Package                       |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64    |


Scenario: Construct query and upgrade multiple packages and their dependencies
  Given I use repository "dnf-ci-fedora-updates"
   When I execute python libdnf5 api script with setup
        """
        goal = libdnf5.base.Goal(base)
        goal.add_rpm_upgrade("glibc")
        goal.add_rpm_upgrade("flac")
        execute_transaction(goal, "upgrade packages and their dependencies")
        """
   Then the exit code is 0
    And stdout is
        """
        flac-1.3.3-3.fc29.x86_64 : Upgrade
        glibc-2.28-26.fc29.x86_64 : Upgrade
        glibc-common-2.28-26.fc29.x86_64 : Upgrade
        glibc-all-langpacks-2.28-26.fc29.x86_64 : Upgrade
        flac-1.3.2-8.fc29.x86_64 : Replaced
        glibc-2.28-9.fc29.x86_64 : Replaced
        glibc-all-langpacks-2.28-9.fc29.x86_64 : Replaced
        glibc-common-2.28-9.fc29.x86_64 : Replaced
        """
    And RPMDB Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |


Scenario: Construct query and downgrade flac package
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade flac"
   Then the exit code is 0
   When I execute python libdnf5 api script with setup
        """
        goal = libdnf5.base.Goal(base)
        goal.add_rpm_downgrade("flac")
        execute_transaction(goal, "downgrade a package without dependencies")
        """
   Then the exit code is 0
    And stdout is
        """
        flac-1.3.3-2.fc29.x86_64 : Downgrade
        flac-1.3.3-3.fc29.x86_64 : Replaced
        """
    And RPMDB Transaction is following
        | Action        | Package                       |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64    |


Scenario: Construct query and downgrade multiple packages and their dependencies
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade glibc flac"
   Then the exit code is 0
   When I execute python libdnf5 api script with setup
        """
        goal = libdnf5.base.Goal(base)
        goal.add_rpm_downgrade("flac")
        goal.add_rpm_downgrade("glibc")
        execute_transaction(goal, "downgrade packages and their dependencies")
        """
   Then the exit code is 0
    And stdout is
        """
        glibc-2.28-9.fc29.x86_64 : Downgrade
        flac-1.3.3-2.fc29.x86_64 : Downgrade
        glibc-common-2.28-9.fc29.x86_64 : Downgrade
        glibc-all-langpacks-2.28-9.fc29.x86_64 : Downgrade
        flac-1.3.3-3.fc29.x86_64 : Replaced
        glibc-2.28-26.fc29.x86_64 : Replaced
        glibc-all-langpacks-2.28-26.fc29.x86_64 : Replaced
        glibc-common-2.28-26.fc29.x86_64 : Replaced
        """
    And RPMDB Transaction is following
        | Action        | Package                                   |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64         |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64                |
