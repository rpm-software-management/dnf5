Feature: Test upgrading installonly packages


Background:
  Given I use repository "dnf-ci-fedora"


@bz1668256 @bz1616191 @bz1639429
Scenario: Install multiple versions of an installonly package with a limit of 2
  Given I set config option "installonly_limit" to "2"
   When I execute dnf with args "install kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
   When I execute dnf with args "upgrade kernel-core"
   Then the exit code is 0
   Then stderr does not contain "cannot install both"
    And Transaction is empty
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64  |
        | unchanged     | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | remove        | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package state is
        | package                             | reason | from_repo                     |
        | kernel-core-4.19.15-300.fc29.x86_64 | User   | dnf-ci-fedora-updates         |
        | kernel-core-4.20.6-300.fc29.x86_64  | User   | dnf-ci-fedora-updates-testing |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository                    |
        | Install | kernel-core-0:4.20.6-300.fc29.x86_64  | User   | dnf-ci-fedora-updates-testing |
        | Remove  | kernel-core-0:4.18.16-300.fc29.x86_64 | User   | @System                       |


Scenario: Install and remove multiple versions of an installonly package
  Given I set config option "installonly_limit" to "2"
   When I execute dnf with args "install kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package state is
        | package                             | reason | from_repo             |
        | kernel-core-4.18.16-300.fc29.x86_64 | User   | dnf-ci-fedora         |
        | kernel-core-4.19.15-300.fc29.x86_64 | User   | dnf-ci-fedora-updates |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository            |
        | Install | kernel-core-0:4.19.15-300.fc29.x86_64 | User   | dnf-ci-fedora-updates |
   When I execute dnf with args "remove kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | remove        | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package state is
        | package | reason | from_repo |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository |
        | Remove  | kernel-core-0:4.18.16-300.fc29.x86_64 | User   | @System    |
        | Remove  | kernel-core-0:4.19.15-300.fc29.x86_64 | User   | @System    |


@bz1769788
Scenario: Install multiple versions of an installonly package and keep reason
   When I execute dnf with args "install kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package state is
        | package                             | reason | from_repo             |
        | kernel-core-4.18.16-300.fc29.x86_64 | User   | dnf-ci-fedora         |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository    |
        | Install | kernel-core-0:4.18.16-300.fc29.x86_64 | User   | dnf-ci-fedora |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade --no-best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package state is
        | package                             | reason | from_repo             |
        | kernel-core-4.18.16-300.fc29.x86_64 | User   | dnf-ci-fedora         |
        | kernel-core-4.19.15-300.fc29.x86_64 | User   | dnf-ci-fedora-updates |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository            |
        | Install | kernel-core-0:4.19.15-300.fc29.x86_64 | User   | dnf-ci-fedora-updates |
   When I execute dnf with args "autoremove"
   Then the exit code is 0
    And Transaction is empty


@xfail
# Depends on issue: https://github.com/rpm-software-management/dnf5/issues/762
@bz1774670
Scenario: Remove all installonly packages but keep the latest
   When I execute dnf with args "install kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                  |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64     |
        | unchanged     | kernel-core-0:4.19.15-300.fc29.x86_64    |
        | unchanged        | kernel-core-0:4.18.16-300.fc29.x86_64 |
   When I execute dnf with args "remove --oldinstallonly"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                  |
        | unchanged       | kernel-core-0:4.20.6-300.fc29.x86_64   |
        | remove        | kernel-core-0:4.19.15-300.fc29.x86_64    |
        | remove        | kernel-core-0:4.18.16-300.fc29.x86_64    |


@xfail
# Depends on issue: https://github.com/rpm-software-management/dnf5/issues/762
@bz1774670
@no_installroot
@destructive
Scenario: Remove all installonly packages but keep the latest and running kernel-core-0:4.18.16-300.fc29.x86_64
  Given I use repository "dnf-ci-fedora"
    And I fake kernel release to "4.18.16-300.fc29.x86_64"
   When I execute dnf with args "install kernel-core --repofrompath=r,{context.dnf.repos[dnf-ci-fedora].path} --repo=r --nogpgcheck"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade --repofrompath=r,{context.dnf.repos[dnf-ci-fedora-updates].path} --repo=r --nogpgcheck kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "upgrade --repofrompath=r,{context.dnf.repos[dnf-ci-fedora-updates-testing].path} --repo=r --nogpgcheck kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                  |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64     |
        | unchanged     | kernel-core-0:4.19.15-300.fc29.x86_64    |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64    |
   When I execute dnf with args "remove --oldinstallonly"
   Then the exit code is 0
    And Transaction is following
        | Action          | Package                                  |
        | unchanged       | kernel-core-0:4.20.6-300.fc29.x86_64     |
        | remove          | kernel-core-0:4.19.15-300.fc29.x86_64    |
        | unchanged       | kernel-core-0:4.18.16-300.fc29.x86_64   |


@bz1934499
@bz1921063
Scenario: Do not autoremove kernel after upgrade with --best
  Given I successfully execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/kernel-core-4.18.16-300.fc29.x86_64.rpm"
   Then package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | External User   |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade --best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | External User   |
        | kernel-core-4.19.15-300.fc29.x86_64    | External User   |
    And package state is
        | package                             | reason        | from_repo             |
        | kernel-core-4.19.15-300.fc29.x86_64 | External User | dnf-ci-fedora-updates |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason        | repository            |
        | Install | kernel-core-0:4.19.15-300.fc29.x86_64 | External User | dnf-ci-fedora-updates |
   When I execute dnf with args "autoremove"
   Then the exit code is 0
    And Transaction is empty


@bz1934499
@bz1921063
Scenario: Do not autoremove kernel after upgrade with --no-best
  Given I successfully execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/kernel-core-4.18.16-300.fc29.x86_64.rpm"
   Then package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | External User   |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade --no-best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
  #  Also valid result can be unknown reason
    And package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | External User   |
        | kernel-core-4.19.15-300.fc29.x86_64    | External User   |
    And package state is
        | package                             | reason        | from_repo             |
        | kernel-core-4.19.15-300.fc29.x86_64 | External User | dnf-ci-fedora-updates |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason        | repository            |
        | Install | kernel-core-0:4.19.15-300.fc29.x86_64 | External User | dnf-ci-fedora-updates |
   When I execute dnf with args "autoremove"
   Then the exit code is 0
    And Transaction is empty


@bz1934499
@bz1921063
Scenario: Do not remove or change reason after remove of one of installonly packages
   When I execute dnf with args "install kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
   When I execute dnf with args "upgrade kernel-core"
   Then the exit code is 0
    And package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | User            |
        | kernel-core-4.19.15-300.fc29.x86_64    | User            |
    And package state is
        | package                             | reason | from_repo             |
        | kernel-core-4.18.16-300.fc29.x86_64 | User   | dnf-ci-fedora         |
        | kernel-core-4.19.15-300.fc29.x86_64 | User   | dnf-ci-fedora-updates |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository            |
        | Install | kernel-core-0:4.19.15-300.fc29.x86_64 | User   | dnf-ci-fedora-updates |
   When I execute dnf with args "remove kernel-core-0:4.19.15-300.fc29.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | User            |
    And package state is
        | package                             | reason | from_repo             |
        | kernel-core-4.18.16-300.fc29.x86_64 | User   | dnf-ci-fedora         |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository |
        | Remove  | kernel-core-0:4.19.15-300.fc29.x86_64 | User   | @System    |


@bz1934499
@bz1921063
Scenario: Keep reason for installonly packages
   When I execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/kernel-core-4.18.16-300.fc29.x86_64.rpm {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/kernel-core-4.19.15-300.fc29.x86_64.rpm"
   Then the exit code is 0

    And package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | External User   |
        | kernel-core-4.19.15-300.fc29.x86_64    | External User   |
  When I execute dnf with args "remove kernel-core-0:4.19.15-300.fc29.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |
    And package reasons are
        | Package                                | Reason          |
        | kernel-core-4.18.16-300.fc29.x86_64    | External User   |
    And package state is
        | package | reason | from_repo |
    And dnf5 transaction items for transaction "last" are
        | action  | package                               | reason | repository |
        | Remove  | kernel-core-0:4.19.15-300.fc29.x86_64 | User   | @System    |
   When I execute dnf with args "autoremove"
   Then the exit code is 0
    And Transaction is empty


@xfail
# https://github.com/rpm-software-management/dnf5/issues/1789
@bz1926261
Scenario: Value 1 of installonly_limit config option is not allowed
  Given I configure dnf with
        | key               | value     |
        | installonly_limit | 1         |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And stderr matches line by line
    """
    Invalid configuration value: installonly_limit=1 in .*/etc/dnf/dnf.conf; value 1 is not allowed
    """


@xfail
# https://github.com/rpm-software-management/dnf5/issues/1789
# TODO(lukash) dnf5 doesn't seem to implement the limit lower bound and accepts installonly_limit = 1
# also, the rpmdb check seems to not work correctly for this case, since it's passing without an
# error even if the older version is being removed
@bz1926261
Scenario: Kernel upgrade does not fail when installonly_limit=1 (default value is used instead of invalid 1)
  Given I configure dnf with
        | key               | value     |
        | installonly_limit | 1         |
    And I successfully execute dnf with args "install kernel-core"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | unchanged     | kernel-core-0:4.18.16-300.fc29.x86_64 |


@bz2221308 @gh_dnf5_720
Scenario Outline: Dnf can downgrade kernel using "<command>" command.
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install kernel-core-4.19.15-300.fc29.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
   When I execute dnf with args "<command> <args>"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |

Examples:
    | command       | args                                  |
    | downgrade     | kernel-core                           |
    | install       | kernel-core-4.18.16-300.fc29.x86_64   |


@bz2163474
Scenario: Do not bypass installonly limit (2) when installing kernel-core through provide
  Given I set config option "installonly_limit" to "2"
    And I successfully execute dnf with args "install kernel"
    And I use repository "dnf-ci-fedora-updates"
    And I successfully execute dnf with args "upgrade"
    And I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "install kernel-core-uname-r"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                  |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64     |
        | unchanged     | kernel-0:4.19.15-300.fc29.x86_64         |
        | unchanged     | kernel-core-0:4.19.15-300.fc29.x86_64    |
        | unchanged     | kernel-modules-0:4.19.15-300.fc29.x86_64 |
        | remove-dep    | kernel-0:4.18.16-300.fc29.x86_64         |
        | remove        | kernel-core-0:4.18.16-300.fc29.x86_64    |
        | remove-dep    | kernel-modules-0:4.18.16-300.fc29.x86_64 |


@bz2163474
Scenario: Do not bypass installonly limit (default 3) when installing kernel-core through provide
  Given I drop repository "dnf-ci-fedora"
    And I use repository "kernel"
    And I successfully execute dnf with args "install kernel-1.0.0"
    And I successfully execute dnf with args "install kernel-2.0.0"
    And I successfully execute dnf with args "install kernel-3.0.0"
   When I execute dnf with args "install kernel-core-uname-r"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                              |
        | install       | kernel-core-0:4.0.0-1.fc29.x86_64    |
        | unchanged     | kernel-0:2.0.0-1.fc29.x86_64         |
        | unchanged     | kernel-core-0:2.0.0-1.fc29.x86_64    |
        | unchanged     | kernel-modules-0:2.0.0-1.fc29.x86_64 |
        | remove-dep    | kernel-0:1.0.0-1.fc29.x86_64         |
        | remove        | kernel-core-0:1.0.0-1.fc29.x86_64    |
        | remove-dep    | kernel-modules-0:1.0.0-1.fc29.x86_64 |


@bz2263675
Scenario: Handle over-limit custom kernel without installonlypkg(kernel) provide
  Given I drop repository "dnf-ci-fedora"
    And I use repository "kernel-custom"
    And I successfully execute dnf with args "install kernel-6.5.10"
    And I successfully execute dnf with args "install kernel-6.5.12"
    And I successfully execute dnf with args "install kernel-6.6.3"
    And I successfully execute rpm with args "-i {context.dnf.fixturesdir}/repos/kernel-custom/x86_64/kernel-6.7.0+-7.x86_64.rpm"
   When I execute dnf with args "upgrade 'kernel*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | kernel-0:6.7.4-200.fc29.x86_64  |
        | remove        | kernel-0:6.5.10-300.fc29.x86_64 |
        | remove        | kernel-0:6.5.12-300.fc29.x86_64 |
