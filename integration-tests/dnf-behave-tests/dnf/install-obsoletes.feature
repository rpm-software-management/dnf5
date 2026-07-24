Feature: Install an obsoleted RPM


Scenario: Install an obsoleted RPM
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install glibc-profile"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-profile-0:2.3.1-10.x86_64           |
    And package state is
        | package                       | reason | from_repo         |
        | glibc-profile-2.3.1-10.x86_64 | User   | dnf-ci-thirdparty |
    And dnf5 transaction items for transaction "last" are
        | action  | package                         | reason     | repository        |
        | Install | glibc-profile-0:2.3.1-10.x86_64 | User       | dnf-ci-thirdparty |


Scenario: Install an obsoleted RPM when the obsoleting RPM is available
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install glibc-profile"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
    And package state is
        | package                                | reason     | from_repo     |
        | glibc-2.28-9.fc29.x86_64               | User       | dnf-ci-fedora |
        | setup-2.12.1-1.fc29.noarch             | Dependency | dnf-ci-fedora |
        | filesystem-3.9-2.fc29.x86_64           | Dependency | dnf-ci-fedora |
        | basesystem-11-6.fc29.noarch            | Dependency | dnf-ci-fedora |
        | glibc-common-2.28-9.fc29.x86_64        | Dependency | dnf-ci-fedora |
        | glibc-all-langpacks-2.28-9.fc29.x86_64 | Dependency | dnf-ci-fedora |
    And dnf5 transaction items for transaction "last" are
        | action  | package                                  | reason     | repository    |
        | Install | glibc-0:2.28-9.fc29.x86_64               | User       | dnf-ci-fedora |
        | Install | basesystem-0:11-6.fc29.noarch            | Dependency | dnf-ci-fedora |
        | Install | glibc-common-0:2.28-9.fc29.x86_64        | Dependency | dnf-ci-fedora |
        | Install | filesystem-0:3.9-2.fc29.x86_64           | Dependency | dnf-ci-fedora |
        | Install | setup-0:2.12.1-1.fc29.noarch             | Dependency | dnf-ci-fedora |
        | Install | glibc-all-langpacks-0:2.28-9.fc29.x86_64 | Dependency | dnf-ci-fedora |


@bz1672618
@xfail
# https://github.com/rpm-software-management/dnf5/issues/1783
Scenario: Upgrading obsoleted package by its obsoleter keeps userinstalled=false (with --best)
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install glibc-profile"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-profile-0:2.3.1-10.x86_64           |
   When I execute dnf with args "mark dependency glibc-profile"
   Then the exit code is 0
   When I execute dnf with args "repoquery --userinstalled"
   Then the exit code is 0
    And stdout is empty
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "upgrade --best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install-dep   | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
        | obsoleted     | glibc-profile-0:2.3.1-10.x86_64           |
   When I execute dnf with args "repoquery --userinstalled"
   Then the exit code is 0
    And stdout is empty


@bz1672618
Scenario: Upgrading obsoleted package by its obsoleter keeps userinstalled=false (with --nobest)
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install glibc-profile"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-profile-0:2.3.1-10.x86_64           |
   When I execute dnf with args "mark dependency glibc-profile"
   Then the exit code is 0
   When I execute dnf with args "repoquery --userinstalled"
   Then the exit code is 0
    And stdout is empty
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "upgrade --nobest"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install-dep   | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
        | obsoleted     | glibc-profile-0:2.3.1-10.x86_64           |
   When I execute dnf with args "repoquery --userinstalled"
   Then the exit code is 0
    And stdout is empty

Scenario: Install obsoleting package and inherit the best reason - user
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install glibc-profile"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-profile-0:2.3.1-10.x86_64           |
   When I execute dnf with args "mark dependency glibc-profile"
   Then the exit code is 0
   When I execute dnf with args "repoquery --userinstalled"
   Then the exit code is 0
    And stdout is empty
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc-0:2.28-9.fc29.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
        | obsoleted     | glibc-profile-0:2.3.1-10.x86_64           |
   When I execute dnf with args "repoquery --userinstalled"
   Then the exit code is 0
    And stdout is
        """
        glibc-0:2.28-9.fc29.x86_64
        """


# dnf-3 fails this test, it incorrectly installs the new packages
# as dependencies.
Scenario: install all obsoleters (as user installed), obsoles behavior specified by `SOLVER_FLAG_YUM_OBSOLETES` libsolv flag
  Given I use repository "obsoletes-yum"
    And I successfully execute dnf with args "install wood-1.0"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                    |
        | install | copper-0:1.0-1.fc29.x86_64 |
        | install | iron-0:1.0-1.fc29.x86_64   |
        | upgrade | wood-0:2.0-1.fc29.x86_64   |
   When I execute dnf with args "rq --installed --qf '%{{name}} - %{{reason}}\n'"
   Then stdout is
   """
   copper - User
   iron - User
   wood - User
   """
