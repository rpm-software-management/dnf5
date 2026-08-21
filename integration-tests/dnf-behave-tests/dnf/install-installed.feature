Feature: Install installed RPMs


Scenario: Install installed RPM when upgrade is available with --best
  Given I use repository "dnf-ci-fedora"
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
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc --best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |


@dnf5daemon
@bz1670776 @bz1671683
Scenario: Install installed RPM when upgrade is available with best=True (in dnf.conf)
  Given I use repository "dnf-ci-fedora"
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
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |


@dnf5daemon
@bz1670776 @bz1671683
Scenario: Install installed RPM when upgrade is available with best=False (in dnf.conf)
  Given I use repository "dnf-ci-fedora"
  Given I configure dnf with
        | key  | value |
        | best | False |
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
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is empty


@bz1670776 @bz1671683
Scenario: Install installed RPM when upgrade is available with option --nobest
  Given I use repository "dnf-ci-fedora"
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
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc --nobest"
   Then the exit code is 0
    And Transaction is empty


@dnf5daemon
Scenario: Install installed RPM when downgrade is available
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-26.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64        |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is empty


@dnf5daemon
Scenario: Install installed RPM by NEVR when upgrade is available
  Given I use repository "dnf-ci-fedora"
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
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc-0:2.28-9.fc29"
   Then the exit code is 0
    And Transaction is empty


@dnf5daemon
Scenario: Install installed RPM by NEVR when downgrade is available
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-26.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64        |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
   When I execute dnf with args "install glibc-0:2.28-26.fc29"
   Then the exit code is 0
    And Transaction is empty
