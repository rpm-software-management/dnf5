Feature: Install different version of installed RPMs

@dnf5daemon
Scenario: Install higher versions of installed RPMs
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-0:2.28-9.fc29.x86_64                 |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | basesystem-0:11-6.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64          |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64   |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc-2.28-26.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64                |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64         |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |

Scenario: Install lower versions of installed RPMs
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-0:2.28-26.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | basesystem-0:11-6.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
   When I execute dnf with args "install glibc-2.28-9.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                 |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64          |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64   |

Scenario: Install lower versions of installed RPMs with --allow-downgrade
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-0:2.28-26.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | basesystem-0:11-6.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
   When I execute dnf with args "install glibc-2.28-9.fc29 --allow-downgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                 |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64          |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64   |

Scenario: Install lower versions of installed RPMs and no-allow-downgrade
#  expected to fail due to operation requires downgrade dependencies
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-0:2.28-26.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | basesystem-0:11-6.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
   When I execute dnf with args "install glibc-2.28-9.fc29 --no-allow-downgrade"
   Then the exit code is 1
