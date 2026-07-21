Feature: Mark command


Scenario Outline: Marking non-existent package as <type> fails
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "mark <type> nosuchpkg"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    No match for argument: nosuchpkg
    """

Examples:
        | type        |
        | user        |
        | dependency  |
        | weak        |


Scenario: Marking as group for non-existent package or non-existent group fails
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "mark group dnf-ci-testgroup nosuchpkg"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    No match for argument: nosuchpkg
    """
   When I execute dnf with args "install lame"
    And I execute dnf with args "mark group nosuchgrp lame"
   Then the exit code is 1
    And stderr contains lines
    """
    Group state for "nosuchgrp" not found.
    """


Scenario: Marking available but not installed package fails
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "mark user lame"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    No match for argument: lame
    """


Scenario: Marking as dependency a list of pkgs when one of them is not available fails
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install lame"
    And I execute dnf with args "mark dependency lame nosuchpkg"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    No match for argument: nosuchpkg
    """
    And package reasons are
        | Package                       | Reason     |
        | lame-3.100-4.fc29.x86_64      | User       |
        | lame-libs-3.100-4.fc29.x86_64 | Dependency |


Scenario: Marking as dependency a list of pkgs when one of them is not available passes with --skip-unavailable
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install lame"
    And I execute dnf with args "mark --skip-unavailable dependency lame nosuchpkg"
   Then the exit code is 0
    And stderr contains lines
        """
        No match for argument: nosuchpkg
        """
    And package reasons are
        | Package                       | Reason     |
        | lame-3.100-4.fc29.x86_64      | Dependency |
        | lame-libs-3.100-4.fc29.x86_64 | Dependency |


Scenario Outline: Mark user installed package as <type>
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install lame"
   Then the exit code is 0
   When I execute dnf with args "mark <arg> lame"
   Then the exit code is 0
    And package reasons are
        | Package                      | Reason    |
        | lame-3.100-4.fc29.x86_64     | <type>    |
        | lame-libs-3.100-4.fc29.x86_64 | Dependency |

Examples:
        | arg        | type            |
        | dependency | Dependency      |
        | weak       | Weak Dependency |


Scenario Outline: Mark package installed as dependency as <type>
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And package reasons are
        | Package                      | Reason     |
        | filesystem-3.9-2.fc29.x86_64 | User       |
        | setup-2.12.1-1.fc29.noarch   | Dependency |
   When I execute dnf with args "mark <arg> setup"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason |
        | filesystem-3.9-2.fc29.x86_64   | User   |
        | setup-2.12.1-1.fc29.noarch   | <type> |

Examples:
        | arg  | type            |
        | user | User            |
        | weak | Weak Dependency |


Scenario: Mark package as the same reason it currently has
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install lame"
   Then the exit code is 0
    And package reasons are
        | Package                       | Reason     |
        | lame-3.100-4.fc29.x86_64      | User       |
        | lame-libs-3.100-4.fc29.x86_64 | Dependency |
   When I execute dnf with args "mark user lame"
   Then the exit code is 0
    And stdout is
        """
        Nothing to do.
        """
    And stderr is
        """
        <REPOSYNC>
        Package "lame-3.100-4.fc29.x86_64" is already installed with reason "User".
        """


Scenario: Mark user installed package as group
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "install lame"
    And I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And package reasons are
        | Package                       | Reason     |
        | filesystem-3.9-2.fc29.x86_64  | Group      |
        | lame-3.100-4.fc29.x86_64      | User       |
        | lame-libs-3.100-4.fc29.x86_64 | Dependency |
        | setup-2.12.1-1.fc29.noarch    | Dependency |
   When I execute dnf with args "mark group dnf-ci-testgroup lame"
   Then the exit code is 0
    And package reasons are
        | Package                       | Reason     |
        | filesystem-3.9-2.fc29.x86_64  | Group      |
        | lame-3.100-4.fc29.x86_64      | Group      |
        | lame-libs-3.100-4.fc29.x86_64 | Dependency |
        | setup-2.12.1-1.fc29.noarch    | Dependency |


Scenario: Mark group installed package as user and back again
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
    And I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | Group      |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
   When I execute dnf with args "mark user lame"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | User       |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
   When I execute dnf with args "mark group dnf-ci-testgroup lame"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | Group      |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
   When I execute dnf with args "mark group dnf-ci-testgroup lame"
   Then the exit code is 0
    And stdout does not contain "User -> Group"
    And stderr is
    """
    Package "lame-3.100-4.fc29.x86_64" is already installed with reason "Group".
    """


# https://github.com/rpm-software-management/dnf5/issues/1976
Scenario: Mark group installed package as dependency and back again
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
    And I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | Group      |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
   When I execute dnf with args "mark dependency lame"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | Dependency |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
   When I execute dnf with args "mark group dnf-ci-testgroup lame"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | Group      |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
   When I execute dnf with args "mark group dnf-ci-testgroup lame"
   Then the exit code is 0
    And stdout does not contain "Dependency -> Group"
    And stderr is
    """
    Package "lame-3.100-4.fc29.x86_64" is already installed with reason "Group".
    """


Scenario: Marking dependency as user-installed should not remove it automatically
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
   When I execute dnf with args "mark user setup"
   Then the exit code is 0
   When I execute dnf with args "remove filesystem"
   Then the exit code is 0
   And Transaction is following
        | Action        | Package                                   |
        | remove        | filesystem-0:3.9-2.fc29.x86_64            |
        | unchanged     | setup-0:2.12.1-1.fc29.noarch              |
   When I execute dnf with args "remove setup"
   Then the exit code is 0
   And Transaction is following
        | Action        | Package                                   |
        | remove        | setup-0:2.12.1-1.fc29.noarch              |


@bz2046581
Scenario: Marking installed package when history DB is not on the system (deleted or not created yet)
   When I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/wget-1.19.6-5.fc29.x86_64.rpm"
   Then the exit code is 0
    And package reasons are
        | Package                      | Reason         |
        | wget-1.19.6-5.fc29.x86_64    | External User  |
   When I execute dnf with args "mark user wget"
   Then the exit code is 0
    And package reasons are
        | Package                      | Reason        |
        | wget-1.19.6-5.fc29.x86_64    | User          |


Scenario: Marking toplevel package as dependency should not remove shared dependencies on autoremove
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install nss_hesiod libnsl"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | libnsl-0:2.28-9.fc29.x86_64               |
        | install       | nss_hesiod-0:2.28-9.fc29.x86_64           |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
   When I execute dnf with args "mark dependency libnsl"
   Then the exit code is 0
   When I execute dnf with args "autoremove"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | libnsl-0:2.28-9.fc29.x86_64               |


Scenario: Mark package as group installed for multiple groups
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
    And I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | Group      |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
  Given I execute dnf with args "group install --no-packages cqrlib-non-devel"
   Then the exit code is 0
   When I execute dnf with args "mark group cqrlib-non-devel lame"
   Then the exit code is 0
    And package reasons are
        | Package                        | Reason     |
        | filesystem-3.9-2.fc29.x86_64   | Group      |
        | lame-3.100-4.fc29.x86_64       | Group      |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
        | setup-2.12.1-1.fc29.noarch     | Dependency |
   When I execute dnf with args "group remove dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | filesystem-0:3.9-2.fc29.x86_64            |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch              |
        | group-remove  | DNF-CI-Testgroup                          |
    And package reasons are
        | Package                        | Reason     |
        | lame-3.100-4.fc29.x86_64       | Group      |
        | lame-libs-3.100-4.fc29.x86_64  | Dependency |
   When I execute dnf with args "group remove cqrlib-non-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | lame-3.100-4.fc29.x86_64                  |
        | remove-unused | lame-libs-3.100-4.fc29.x86_64             |
        | group-remove  | CQRlib-non-devel                          |
