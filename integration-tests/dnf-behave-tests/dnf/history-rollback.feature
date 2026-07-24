Feature: Transaction history rollback


Background:
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
    And I successfully execute dnf with args "install basesystem"
    And I successfully execute dnf with args "install glibc-2.28-26.fc29.x86_64"
    And I successfully execute dnf with args "downgrade glibc"
    And I successfully execute dnf with args "upgrade glibc"


Scenario: Rollback
   When I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | remove        | glibc-2.28-26.fc29.x86_64                  |
        | remove-unused | glibc-all-langpacks-2.28-26.fc29.x86_64    |
        | remove-unused | glibc-common-2.28-26.fc29.x86_64           |


Scenario: Multiple rollbacks
   When I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | remove        | glibc-2.28-26.fc29.x86_64                  |
        | remove-unused | glibc-all-langpacks-2.28-26.fc29.x86_64    |
        | remove-unused | glibc-common-2.28-26.fc29.x86_64           |
   When I execute dnf with args "history rollback 2"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | glibc-2.28-26.fc29.x86_64                  |
        | install-dep   | glibc-all-langpacks-2.28-26.fc29.x86_64    |
        | install-dep   | glibc-common-2.28-26.fc29.x86_64           |


Scenario: Rollback a transaction with a package that is no longer available
   When I execute dnf with args "history rollback 1 -x glibc-2.28-26.fc29.x86_64"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Cannot perform Remove action because 'glibc-2.28-26.fc29.x86_64' matches only excluded packages.
        Problem: installed package glibc-2.28-26.fc29.x86_64 requires glibc-common = 2.28-26.fc29, but none of the providers can be installed
          - conflicting requests
          - problem with installed package
        """


Scenario: Rollback install of multiple installonly pkgs
  Given I use repository "installonly"
    And I configure dnf with
        | key               | value        |
        | installonlypkgs   | installonlyB |
        | installonly_limit | 2            |
    And I successfully execute dnf with args "install installonlyB-1.0 installonlyB-2.0"
   When I execute dnf with args "history rollback last-1"
   Then the exit code is 0
    And Transaction is following
        | Action | Package                   |
        | remove | installonlyB-1.0-1.x86_64 |
        | remove | installonlyB-2.0-1.x86_64 |


Scenario: Rollback rollback of install of multiple installonly pkgs
  Given I use repository "installonly"
    And I configure dnf with
        | key               | value        |
        | installonlypkgs   | installonlyB |
        | installonly_limit | 2            |
    And I successfully execute dnf with args "install installonlyB-1.0 installonlyB-2.0"
    And I successfully execute dnf with args "history rollback last-1"
   When I execute dnf with args "history rollback last-1"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                   |
        | install | installonlyB-1.0-1.x86_64 |
        | install | installonlyB-2.0-1.x86_64 |


Scenario: Rollback upgrade with obsolete
  Given I use repository "dnf-ci-obsoletes"
    And I successfully execute dnf with args "install PackageB-2.0"
    And I successfully execute dnf with args "upgrade"
   When I execute dnf with args "history rollback last-1"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                         |
        | install | PackageB-2.0-1.x86_64           |
        | remove  | PackageB-Obsoleter-1.0-1.x86_64 |


Scenario: Rollback upgrade with two obsoleters
  Given I use repository "dnf-ci-obsoletes"
    And I successfully execute dnf with args "install PackageF-1.0"
    And I successfully execute dnf with args "upgrade"
   When I execute dnf with args "history rollback last-1"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                                |
        | install | PackageF-1.0-1.x86_64                  |
        | remove  | PackageF-Obsoleter-3.0-1.x86_64        |
        | remove  | PackageF-Obsoleter-Second-3.0-1.x86_64 |


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/2223
Scenario: Rollback reinstall action
  Given I successfully execute dnf with args "reinstall glibc"
   When I execute dnf with args "history rollback last-1"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | reinstall | glibc-2.28-26.fc29.x86_64 |
    And stderr does not contain "Transaction merge error"


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/2223
Scenario: Rollback install + reinstall action
  Given I successfully execute dnf with args "install wget"
    And I successfully execute dnf with args "reinstall wget"
   When I execute dnf with args "history rollback last-2"
   Then the exit code is 0
    And Transaction is following
        | Action | Package                   |
        | remove | wget-1.19.6-5.fc29.x86_64 |
    And stderr does not contain "Transaction merge error"
